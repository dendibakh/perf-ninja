#!/usr/bin/env python3
"""Read-only official CI monitoring for Performance Ninja submissions."""

from __future__ import annotations

import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence

from pn_submit import (
    COMMIT_PATTERN,
    OFFICIAL_REPOSITORY,
    CommandError,
    current_branch,
    gh_json,
    git_text,
    github_repository_access,
)


EXPECTED_WORKFLOWS = (
    "Benchmark on Linux + Alderlake",
    "Benchmark on Mac OS + M1",
    "Benchmark on Windows + Zen3",
)


class CiFailed(CommandError):
    """Official CI completed but at least one benchmark workflow failed."""

    exit_code = 1


class CiPending(CommandError):
    """Official CI has not reached a terminal result yet."""

    exit_code = 8


@dataclass(frozen=True)
class CiRun:
    database_id: int
    workflow: str
    status: str
    conclusion: str
    url: str
    attempt: int
    created_at: str


def resolve_commit(repo: Path, ref: str) -> str:
    if not ref or ref.startswith("-"):
        raise CommandError(f"Invalid commit ref {ref!r}.")
    commit = git_text(
        ["rev-parse", "--verify", "--end-of-options", f"{ref}^{{commit}}"], cwd=repo
    )
    if not COMMIT_PATTERN.fullmatch(commit):
        raise CommandError(f"Git returned an invalid commit for {ref!r}.")
    return commit


def parse_ci_runs(payload: object, commit: str, branch: str) -> dict[str, CiRun]:
    if not isinstance(payload, list):
        raise CommandError("GitHub CLI returned an invalid workflow-run list.")
    selected: dict[str, CiRun] = {}
    for row in payload:
        if not isinstance(row, dict):
            continue
        workflow = row.get("workflowName")
        if (
            workflow not in EXPECTED_WORKFLOWS
            or row.get("headSha") != commit
            or row.get("headBranch") != branch
            or row.get("event") != "push"
        ):
            continue
        try:
            run = CiRun(
                database_id=int(row["databaseId"]),
                workflow=str(workflow),
                status=str(row["status"]),
                conclusion=str(row.get("conclusion") or ""),
                url=str(row["url"]),
                attempt=int(row.get("attempt") or 1),
                created_at=str(row.get("createdAt") or ""),
            )
        except (KeyError, TypeError, ValueError) as error:
            raise CommandError(
                f"GitHub CLI returned an invalid run for {workflow}."
            ) from error
        previous = selected.get(run.workflow)
        if previous is None or (
            run.created_at,
            run.attempt,
            run.database_id,
        ) > (previous.created_at, previous.attempt, previous.database_id):
            selected[run.workflow] = run
    return selected


def list_ci_runs(
    repo: Path, repository: str, commit: str, branch: str
) -> dict[str, CiRun]:
    fields = (
        "databaseId,workflowName,status,conclusion,url,attempt,createdAt,"
        "headSha,headBranch,event"
    )
    payload = gh_json(
        [
            "gh",
            "run",
            "list",
            "--repo",
            repository,
            "--commit",
            commit,
            "--branch",
            branch,
            "--event",
            "push",
            "--limit",
            "30",
            "--json",
            fields,
        ],
        cwd=repo,
    )
    return parse_ci_runs(payload, commit, branch)


def print_ci_status(commit: str, branch: str, runs: dict[str, CiRun]) -> None:
    print(f"\nOfficial CI for {commit[:12]} on {branch}")
    print(f"{'Workflow':<38} {'State':<22} URL")
    print("-" * 100)
    for workflow in EXPECTED_WORKFLOWS:
        run = runs.get(workflow)
        if run is None:
            print(f"{workflow:<38} {'not found':<22} -")
            continue
        state = run.status
        if run.status == "completed":
            state = f"completed/{run.conclusion or 'unknown'}"
        print(f"{workflow:<38} {state:<22} {run.url}")


def ci_signature(runs: dict[str, CiRun]) -> tuple[tuple[object, ...], ...]:
    return tuple(
        (
            workflow,
            runs[workflow].database_id,
            runs[workflow].status,
            runs[workflow].conclusion,
        )
        for workflow in EXPECTED_WORKFLOWS
        if workflow in runs
    )


def show_failed_logs(repo: Path, repository: str, failed: Sequence[CiRun]) -> None:
    for run in failed:
        print(f"\nFailed logs: {run.workflow}")
        completed = subprocess.run(
            [
                "gh",
                "run",
                "view",
                str(run.database_id),
                "--repo",
                repository,
                "--log-failed",
            ],
            cwd=repo,
            text=True,
            check=False,
        )
        if completed.returncode != 0:
            print(f"Could not retrieve logs; inspect {run.url}", file=sys.stderr)


def finish_ci(repo: Path, repository: str, runs: dict[str, CiRun]) -> None:
    failed = tuple(
        runs[workflow]
        for workflow in EXPECTED_WORKFLOWS
        if runs[workflow].conclusion != "success"
    )
    if failed:
        show_failed_logs(repo, repository, failed)
        names = ", ".join(run.workflow for run in failed)
        raise CiFailed(f"Official CI failed: {names}")
    print("Official CI passed on all three benchmark platforms.")


def check_ci(
    repo: Path,
    ref: str,
    *,
    branch: str | None,
    watch: bool,
    timeout_seconds: int,
    interval_seconds: int,
) -> None:
    if timeout_seconds < 1 or interval_seconds < 1:
        raise CommandError("CI timeout and polling interval must be positive seconds.")
    repository, _permission, _default_branch = github_repository_access(repo)
    if repository != OFFICIAL_REPOSITORY:
        raise CommandError(
            f"Official CI is available only for {OFFICIAL_REPOSITORY}, not {repository}."
        )
    resolved_branch = branch or current_branch(repo)
    if not resolved_branch:
        raise CommandError("CI lookup from detached HEAD requires --branch.")
    commit = resolve_commit(repo, ref)
    deadline = time.monotonic() + timeout_seconds
    previous_signature: tuple[tuple[object, ...], ...] | None = None
    while True:
        runs = list_ci_runs(repo, repository, commit, resolved_branch)
        signature = ci_signature(runs)
        if signature != previous_signature:
            print_ci_status(commit, resolved_branch, runs)
            previous_signature = signature
        terminal = len(runs) == len(EXPECTED_WORKFLOWS) and all(
            run.status == "completed" for run in runs.values()
        )
        if terminal:
            finish_ci(repo, repository, runs)
            return
        if not watch:
            raise CiPending(
                "Official CI is pending or has not started; rerun with pn ci --watch."
            )
        if time.monotonic() >= deadline:
            raise CommandError(
                f"Timed out after {timeout_seconds}s waiting for official CI."
            )
        time.sleep(interval_seconds)


def command_ci(repo: Path, args: object) -> None:
    check_ci(
        repo,
        getattr(args, "commit"),
        branch=getattr(args, "branch"),
        watch=getattr(args, "watch"),
        timeout_seconds=getattr(args, "timeout"),
        interval_seconds=getattr(args, "interval"),
    )
