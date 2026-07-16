#!/usr/bin/env python3
"""Safe Git and GitHub submission workflow for Performance Ninja labs."""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from collections.abc import Callable, Sequence
from dataclasses import dataclass
from pathlib import Path
from typing import Mapping, Protocol
from urllib.parse import urlparse


OFFICIAL_REPOSITORY = "dendibakh/perf-ninja"
WRITE_PERMISSIONS = {"WRITE", "MAINTAIN", "ADMIN"}
COMMIT_PATTERN = re.compile(r"[0-9a-fA-F]{40,64}")


class CommandError(RuntimeError):
    """An actionable submission or GitHub command error."""

    exit_code = 2


class SubparserCollection(Protocol):
    def add_parser(self, name: str, **kwargs: object) -> argparse.ArgumentParser: ...


@dataclass(frozen=True)
class SubmissionPlan:
    repo: Path
    relative_lab: Path
    branch: str
    repository: str
    permission: str
    message: str
    changes: tuple[str, ...]
    outgoing_commits: tuple[str, ...]
    has_upstream: bool
    head_commit: str
    fingerprint: str
    patch: str
    origin_push_url: str


@dataclass(frozen=True)
class LabSnapshot:
    tree: str
    patch: str


def positive_int(value: str) -> int:
    parsed = int(value)
    if parsed < 1:
        raise argparse.ArgumentTypeError("value must be at least 1")
    return parsed


def add_parsers(
    subparsers: SubparserCollection,
    add_common: Callable[[str, str], argparse.ArgumentParser],
) -> None:
    submit = add_common("submit", "validate, commit, push, and monitor one lab safely")
    submit.add_argument(
        "--runs", type=positive_int, default=5, help="number of alternating runs"
    )
    submit.add_argument(
        "--baseline", default="main", help="Git ref used as the baseline"
    )
    submit.add_argument(
        "-m", "--message", help="one-line commit message; defaults from the lab name"
    )
    submit.add_argument(
        "--dry-run",
        action="store_true",
        help="run local checks and preview the submission without Git mutations",
    )
    submit.add_argument(
        "--no-watch", action="store_true", help="do not wait for CI after pushing"
    )
    submit.add_argument(
        "-y",
        "--yes",
        action="store_true",
        help="submit without an interactive confirmation",
    )
    submit.add_argument(
        "--ci-timeout",
        type=positive_int,
        default=900,
        metavar="SECONDS",
        help="maximum time to wait for CI (default: 900)",
    )
    submit.add_argument(
        "--ci-interval",
        type=positive_int,
        default=5,
        metavar="SECONDS",
        help="CI polling interval (default: 5)",
    )

    ci = subparsers.add_parser(
        "ci", help="check official CI for an exact commit and branch"
    )
    ci.add_argument("--commit", default="HEAD", help="commit ref to check")
    ci.add_argument(
        "--branch", help="remote branch containing the commit; defaults to current"
    )
    ci.add_argument(
        "--watch", action="store_true", help="wait for all benchmark workflows"
    )
    ci.add_argument(
        "--timeout",
        type=positive_int,
        default=900,
        metavar="SECONDS",
        help="maximum watch time (default: 900)",
    )
    ci.add_argument(
        "--interval",
        type=positive_int,
        default=5,
        metavar="SECONDS",
        help="polling interval (default: 5)",
    )


def command_failure(
    command: Sequence[str], error: subprocess.CalledProcessError
) -> str:
    details = "\n".join(
        part.strip() for part in (error.stdout, error.stderr) if part and part.strip()
    )
    suffix = f"\n{details}" if details else ""
    return f"Command failed: {' '.join(command)}{suffix}"


def run_checked(
    command: Sequence[str],
    *,
    cwd: Path,
    capture_output: bool = False,
    env: Mapping[str, str] | None = None,
) -> subprocess.CompletedProcess[str]:
    try:
        return subprocess.run(
            list(command),
            cwd=cwd,
            text=True,
            capture_output=capture_output,
            check=True,
            env=env,
        )
    except subprocess.CalledProcessError as error:
        raise CommandError(command_failure(command, error)) from error


def git_text(arguments: Sequence[str], *, cwd: Path) -> str:
    return run_checked(["git", *arguments], cwd=cwd, capture_output=True).stdout.strip()


def optional_git_text(arguments: Sequence[str], *, cwd: Path) -> str:
    completed = subprocess.run(
        ["git", *arguments], cwd=cwd, text=True, capture_output=True, check=False
    )
    return completed.stdout.strip() if completed.returncode == 0 else ""


def gh_json(command: Sequence[str], *, cwd: Path) -> object:
    output = run_checked(command, cwd=cwd, capture_output=True).stdout
    try:
        return json.loads(output)
    except json.JSONDecodeError as error:
        raise CommandError(f"GitHub CLI returned invalid JSON: {error}") from error


def current_branch(repo: Path) -> str:
    return optional_git_text(["symbolic-ref", "--quiet", "--short", "HEAD"], cwd=repo)


def current_head(repo: Path) -> str:
    commit = git_text(["rev-parse", "HEAD"], cwd=repo)
    if not COMMIT_PATTERN.fullmatch(commit):
        raise CommandError("Git returned an invalid current commit.")
    return commit


def origin_push_urls(repo: Path) -> tuple[str, ...]:
    output = git_text(["remote", "get-url", "--push", "--all", "origin"], cwd=repo)
    return tuple(line for line in output.splitlines() if line)


def github_slug_from_url(url: str) -> str | None:
    scp_match = re.fullmatch(r"git@github\.com:([^/]+/[^/]+)", url.removesuffix(".git"))
    if scp_match:
        return scp_match.group(1)
    parsed = urlparse(url)
    if parsed.hostname != "github.com":
        return None
    path = parsed.path.strip("/").removesuffix(".git")
    return path if len(path.split("/")) == 2 else None


def origin_push_url(repo: Path) -> str:
    urls = origin_push_urls(repo)
    if len(urls) != 1:
        raise CommandError(
            "origin must have exactly one push destination before submission."
        )
    return urls[0]


def verified_origin_push_url(repo: Path) -> str:
    url = origin_push_url(repo)
    slug = github_slug_from_url(url)
    if slug is None or slug.lower() != OFFICIAL_REPOSITORY.lower():
        raise CommandError(
            f"origin must push to the official repository {OFFICIAL_REPOSITORY}; "
            f"found {url}."
        )
    return url


def github_repository_access(repo: Path) -> tuple[str, str, str]:
    if shutil.which("gh") is None:
        raise CommandError("GitHub CLI is missing; install and authenticate gh first.")
    run_checked(["gh", "auth", "status"], cwd=repo, capture_output=True)
    payload = gh_json(
        [
            "gh",
            "repo",
            "view",
            "--json",
            "nameWithOwner,viewerPermission,defaultBranchRef",
        ],
        cwd=repo,
    )
    if not isinstance(payload, dict):
        raise CommandError("GitHub CLI returned an invalid repository description.")
    repository = payload.get("nameWithOwner")
    permission = payload.get("viewerPermission")
    default_ref = payload.get("defaultBranchRef")
    default_branch = default_ref.get("name") if isinstance(default_ref, dict) else None
    if not all(isinstance(value, str) and value for value in (repository, permission)):
        raise CommandError("GitHub CLI did not report repository access information.")
    if not isinstance(default_branch, str) or not default_branch:
        raise CommandError("GitHub CLI did not report the default branch.")
    return repository, permission, default_branch


def validate_lab_path(relative_lab: Path) -> None:
    if (
        relative_lab.is_absolute()
        or len(relative_lab.parts) < 3
        or relative_lab.parts[0] != "labs"
        or ".." in relative_lab.parts
    ):
        raise CommandError(f"Invalid lab path: {relative_lab}")


def lab_changes(repo: Path, relative_lab: Path) -> tuple[str, ...]:
    output = git_text(
        [
            "status",
            "--short",
            "--untracked-files=all",
            "--",
            str(relative_lab),
        ],
        cwd=repo,
    )
    return tuple(line for line in output.splitlines() if line)


def lab_snapshot(repo: Path, relative_lab: Path) -> LabSnapshot:
    """Build the exact candidate tree and patch without changing the real index."""
    with tempfile.TemporaryDirectory(prefix="pn-index-") as temporary_directory:
        environment = dict(os.environ)
        environment["GIT_INDEX_FILE"] = str(Path(temporary_directory) / "index")
        run_checked(["git", "read-tree", "HEAD"], cwd=repo, env=environment)
        run_checked(
            ["git", "add", "--all", "--", str(relative_lab)],
            cwd=repo,
            env=environment,
        )
        tree = run_checked(
            ["git", "write-tree"],
            cwd=repo,
            capture_output=True,
            env=environment,
        ).stdout.strip()
        patch = run_checked(
            [
                "git",
                "diff",
                "--cached",
                "--no-ext-diff",
                "--binary",
                "--",
                str(relative_lab),
            ],
            cwd=repo,
            capture_output=True,
            env=environment,
        ).stdout.rstrip()
    if not COMMIT_PATTERN.fullmatch(tree):
        raise CommandError("Git returned an invalid candidate tree.")
    return LabSnapshot(tree=tree, patch=patch)


def submission_fingerprint(repo: Path, relative_lab: Path) -> str:
    return lab_snapshot(repo, relative_lab).tree


def lab_patch(repo: Path, relative_lab: Path) -> str:
    return lab_snapshot(repo, relative_lab).patch


def name_status_paths(output: str) -> tuple[str, ...]:
    tokens = output.split("\0")
    if tokens and not tokens[-1]:
        tokens.pop()
    paths: list[str] = []
    index = 0
    while index < len(tokens):
        status = tokens[index]
        index += 1
        path_count = 2 if status[:1] in {"R", "C"} else 1
        if not status or index + path_count > len(tokens):
            raise CommandError("Git returned malformed staged-path information.")
        paths.extend(tokens[index : index + path_count])
        index += path_count
    return tuple(paths)


def staged_paths(repo: Path) -> tuple[str, ...]:
    output = run_checked(
        ["git", "diff", "--cached", "--name-status", "-z", "--"],
        cwd=repo,
        capture_output=True,
    ).stdout
    return name_status_paths(output)


def is_within(path: str, parent: Path) -> bool:
    try:
        Path(path).relative_to(parent)
    except ValueError:
        return Path(path) == parent
    return True


def staged_outside_lab(repo: Path, relative_lab: Path) -> tuple[str, ...]:
    return tuple(
        path for path in staged_paths(repo) if not is_within(path, relative_lab)
    )


def outgoing_commits(
    repo: Path, default_branch: str, branch: str
) -> tuple[tuple[str, ...], bool]:
    upstream = optional_git_text(
        ["rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{upstream}"],
        cwd=repo,
    )
    intended_upstream = f"origin/{branch}"
    has_upstream = upstream == intended_upstream
    if optional_git_text(["rev-parse", "--verify", intended_upstream], cwd=repo):
        base = intended_upstream
    else:
        remote_default = f"origin/{default_branch}"
        base = (
            remote_default
            if optional_git_text(["rev-parse", "--verify", remote_default], cwd=repo)
            else default_branch
        )
    output = git_text(["log", "--oneline", f"{base}..HEAD"], cwd=repo)
    return tuple(line for line in output.splitlines() if line), has_upstream


def submission_message(relative_lab: Path, requested: str | None) -> str:
    message = (
        requested
        if requested is not None
        else f"perf: optimize {relative_lab.name.replace('_', ' ')}"
    )
    if not message.strip() or "\n" in message:
        raise CommandError("The commit message must be one non-empty line.")
    if "[CheckAll]" in message:
        raise CommandError(
            "pn submit sends one lab; remove [CheckAll] to avoid benchmarking every lab."
        )
    return message


def build_submission_plan(
    repo: Path, relative_lab: Path, requested_message: str | None
) -> SubmissionPlan:
    validate_lab_path(relative_lab)
    branch = current_branch(repo)
    if not branch:
        raise CommandError(
            "Submission requires a named branch; detached HEAD is not safe."
        )
    repository, permission, default_branch = github_repository_access(repo)
    if repository != OFFICIAL_REPOSITORY:
        raise CommandError(
            f"Official CI requires the official repository {OFFICIAL_REPOSITORY}; "
            f"this checkout resolves to {repository}."
        )
    if branch == default_branch:
        raise CommandError(
            f"Refusing to submit directly from default branch {branch!r}."
        )
    if permission not in WRITE_PERMISSIONS:
        raise CommandError(
            f"GitHub permission is {permission}; WRITE access is required to submit."
        )
    origin_url = verified_origin_push_url(repo)
    head = current_head(repo)
    message = submission_message(relative_lab, requested_message)
    changes = lab_changes(repo, relative_lab)
    if not changes:
        raise CommandError(f"Nothing to submit in {relative_lab}.")
    outside = staged_outside_lab(repo, relative_lab)
    if outside:
        raise CommandError(
            "Refusing to submit with staged files outside the selected lab:\n"
            + "\n".join(f"- {path}" for path in outside)
        )
    snapshot = lab_snapshot(repo, relative_lab)
    if not snapshot.patch:
        raise CommandError(
            "The submission candidate patch is empty; staged-only lab changes were "
            "left untouched. Restore the intended file contents before submitting."
        )
    fingerprint = snapshot.tree
    patch = snapshot.patch
    if (
        current_branch(repo) != branch
        or current_head(repo) != head
        or verified_origin_push_url(repo) != origin_url
        or submission_fingerprint(repo, relative_lab) != fingerprint
    ):
        raise CommandError(
            "Submission state changed while building the preview; retry."
        )
    outgoing, has_upstream = outgoing_commits(repo, default_branch, branch)
    return SubmissionPlan(
        repo=repo,
        relative_lab=relative_lab,
        branch=branch,
        repository=repository,
        permission=permission,
        message=message,
        changes=changes,
        outgoing_commits=outgoing,
        has_upstream=has_upstream,
        head_commit=head,
        fingerprint=fingerprint,
        patch=patch,
        origin_push_url=origin_url,
    )


def print_submission_plan(plan: SubmissionPlan) -> None:
    print("Official submission preview")
    print(f"  Lab:        {plan.relative_lab}")
    print(f"  Branch:     {plan.branch}")
    print(f"  Repository: {plan.repository} ({plan.permission})")
    print(f"  Push URL:   {plan.origin_push_url}")
    print(f"  Commit:     {plan.message}")
    print("  Lab changes:")
    for change in plan.changes:
        print(f"    {change}")
    if plan.outgoing_commits:
        print("  Existing outgoing commits:")
        for commit in plan.outgoing_commits:
            print(f"    {commit}")
    if not plan.has_upstream:
        print(f"  origin/{plan.branch} will be created or configured as upstream.")
    print("\nExact lab patch:")
    print(plan.patch or "(empty textual patch)")


def confirm_submission() -> bool:
    if not sys.stdin.isatty():
        raise CommandError("Non-interactive submission requires --yes.")
    response = input("Commit and push this lab to official CI? [y/N] ").strip().lower()
    return response in {"y", "yes"}


def verify_staged_scope(plan: SubmissionPlan) -> tuple[str, ...]:
    staged = staged_paths(plan.repo)
    if not staged:
        raise CommandError(f"Nothing was staged for {plan.relative_lab}.")
    outside = tuple(path for path in staged if not is_within(path, plan.relative_lab))
    if outside:
        raise CommandError(
            "Refusing to commit staged files outside the selected lab:\n"
            + "\n".join(f"- {path}" for path in outside)
        )
    return staged


def verify_submission_state(plan: SubmissionPlan) -> None:
    if current_branch(plan.repo) != plan.branch:
        raise CommandError("The current branch changed after the submission preview.")
    if current_head(plan.repo) != plan.head_commit:
        raise CommandError("HEAD changed after the submission preview.")
    if origin_push_url(plan.repo) != plan.origin_push_url:
        raise CommandError("origin's push destination changed after the preview.")
    if submission_fingerprint(plan.repo, plan.relative_lab) != plan.fingerprint:
        raise CommandError("Lab contents changed after the submission preview.")


def submit_changes(plan: SubmissionPlan) -> str:
    verify_submission_state(plan)
    run_checked(["git", "add", "--all", "--", str(plan.relative_lab)], cwd=plan.repo)
    verify_staged_scope(plan)
    verify_submission_state(plan)
    run_checked(["git", "diff", "--cached", "--check"], cwd=plan.repo)
    run_checked(
        ["git", "diff", "--cached", "--stat", "--", str(plan.relative_lab)],
        cwd=plan.repo,
    )
    run_checked(["git", "commit", "-m", plan.message], cwd=plan.repo)
    commit = git_text(["rev-parse", "HEAD"], cwd=plan.repo)
    if not COMMIT_PATTERN.fullmatch(commit):
        raise CommandError("Git returned an invalid commit after submission commit.")
    committed_paths = git_text(
        ["diff-tree", "--no-commit-id", "--name-only", "-r", commit], cwd=plan.repo
    ).splitlines()
    outside = tuple(
        path
        for path in committed_paths
        if path and not is_within(path, plan.relative_lab)
    )
    if outside:
        raise CommandError(
            "The new commit unexpectedly contains files outside the lab; it was not pushed."
        )
    committed_tree = git_text(["rev-parse", f"{commit}^{{tree}}"], cwd=plan.repo)
    if committed_tree != plan.fingerprint:
        raise CommandError(
            "The committed content does not match the reviewed preview; it was not pushed."
        )
    if current_branch(plan.repo) != plan.branch or current_head(plan.repo) != commit:
        raise CommandError("Branch or HEAD changed after commit; it was not pushed.")
    if origin_push_url(plan.repo) != plan.origin_push_url:
        raise CommandError(
            "origin's push destination changed; the commit was not pushed."
        )
    push = ["git", "push", "origin", f"{commit}:refs/heads/{plan.branch}"]
    run_checked(push, cwd=plan.repo)
    if not plan.has_upstream:
        run_checked(
            [
                "git",
                "branch",
                "--set-upstream-to",
                f"origin/{plan.branch}",
                plan.branch,
            ],
            cwd=plan.repo,
        )
    return commit


def command_submit(
    repo: Path,
    relative_lab: Path,
    args: object,
    *,
    compare: Callable[[], None],
) -> None:
    plan = build_submission_plan(repo, relative_lab, getattr(args, "message"))
    print_submission_plan(plan)
    if (
        not getattr(args, "dry_run")
        and not getattr(args, "yes")
        and not sys.stdin.isatty()
    ):
        raise CommandError("Non-interactive submission requires --yes.")
    print("\nRunning local validation and comparison...")
    compare()
    print("Local preflight passed.")
    refreshed_plan = build_submission_plan(repo, relative_lab, getattr(args, "message"))
    if refreshed_plan != plan:
        raise CommandError(
            "Submission state changed during local checks; review and run pn submit again."
        )
    if getattr(args, "dry_run"):
        print("Dry run complete; no files were staged, committed, or pushed.")
        return
    if not getattr(args, "yes") and not confirm_submission():
        print("Submission cancelled; no files were staged, committed, or pushed.")
        return
    commit = submit_changes(plan)
    print(f"Pushed {commit[:12]} to origin/{plan.branch} without force.")
    if getattr(args, "no_watch"):
        print(f"Watch later: pn ci --commit {commit} --branch {plan.branch} --watch")
        return
    from pn_ci import check_ci

    check_ci(
        repo,
        commit,
        branch=plan.branch,
        watch=True,
        timeout_seconds=getattr(args, "ci_timeout"),
        interval_seconds=getattr(args, "ci_interval"),
    )
