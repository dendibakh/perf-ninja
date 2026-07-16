import subprocess
import sys
from dataclasses import replace
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import Mock

import pytest


TOOLS_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS_DIR))

import pn_submit  # noqa: E402
import pn_ci  # noqa: E402


LAB = Path("labs/memory_bound/data_packing")
SHA = "a" * 40
TREE = "b" * 40


def stub_submission_dependencies(
    monkeypatch: pytest.MonkeyPatch,
    *,
    branch: str = "jerefigo",
    repository: str = pn_submit.OFFICIAL_REPOSITORY,
    permission: str = "WRITE",
    default_branch: str = "main",
    changes: tuple[str, ...] = (" M labs/memory_bound/data_packing/solution.h",),
    staged: tuple[str, ...] = (),
) -> None:
    monkeypatch.setattr(pn_submit, "current_branch", lambda _repo: branch)
    monkeypatch.setattr(pn_submit, "current_head", lambda _repo: SHA)
    monkeypatch.setattr(
        pn_submit,
        "lab_snapshot",
        lambda *_: pn_submit.LabSnapshot(TREE, "diff --git a/file b/file"),
    )
    monkeypatch.setattr(pn_submit, "submission_fingerprint", lambda *_: TREE)
    monkeypatch.setattr(
        pn_submit,
        "verified_origin_push_url",
        lambda _repo: "https://github.com/dendibakh/perf-ninja.git",
    )
    monkeypatch.setattr(
        pn_submit,
        "github_repository_access",
        lambda _repo: (repository, permission, default_branch),
    )
    monkeypatch.setattr(pn_submit, "lab_changes", lambda *_: changes)
    monkeypatch.setattr(pn_submit, "staged_paths", lambda _repo: staged)
    monkeypatch.setattr(
        pn_submit, "outgoing_commits", lambda *_: (("1234567 setup",), False)
    )


@pytest.mark.parametrize("branch", ["", "main"])
def test_submission_plan_rejects_detached_or_default_branch(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, branch: str
) -> None:
    stub_submission_dependencies(monkeypatch, branch=branch)

    with pytest.raises(pn_submit.CommandError, match="branch"):
        pn_submit.build_submission_plan(tmp_path, LAB, None)


@pytest.mark.parametrize("permission", ["READ", "TRIAGE"])
def test_submission_plan_requires_write_access(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, permission: str
) -> None:
    stub_submission_dependencies(monkeypatch, permission=permission)

    with pytest.raises(pn_submit.CommandError, match="WRITE"):
        pn_submit.build_submission_plan(tmp_path, LAB, None)


def test_submission_plan_requires_official_repository(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    stub_submission_dependencies(monkeypatch, repository="jerecoder/perf-ninja")

    with pytest.raises(pn_submit.CommandError, match="official repository"):
        pn_submit.build_submission_plan(tmp_path, LAB, None)


def test_submission_plan_rejects_staged_files_outside_lab(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    stub_submission_dependencies(monkeypatch, staged=("labs/misc/warmup/solution.cpp",))

    with pytest.raises(pn_submit.CommandError, match="outside"):
        pn_submit.build_submission_plan(tmp_path, LAB, None)


def test_submission_plan_rejects_empty_lab_diff(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    stub_submission_dependencies(monkeypatch, changes=())

    with pytest.raises(pn_submit.CommandError, match="Nothing to submit"):
        pn_submit.build_submission_plan(tmp_path, LAB, None)


def test_submission_plan_preserves_staged_only_change_when_candidate_is_empty(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    repository = tmp_path / "repo"
    solution = repository / LAB / "solution.h"
    solution.parent.mkdir(parents=True)
    solution.write_text("baseline\n", encoding="utf-8")
    subprocess.run(
        ["git", "init", "-b", "main", repository], check=True, capture_output=True
    )
    git(repository, "config", "user.name", "Test")
    git(repository, "config", "user.email", "test@example.invalid")
    git(repository, "add", ".")
    git(repository, "commit", "-m", "baseline")
    git(repository, "switch", "-c", "jerefigo")
    solution.write_text("staged only\n", encoding="utf-8")
    git(repository, "add", "--", str(LAB))
    git(repository, "restore", "--source=HEAD", "--worktree", "--", str(LAB))
    status_before = git(repository, "status", "--short")
    index_before = git(repository, "write-tree")
    monkeypatch.setattr(
        pn_submit,
        "github_repository_access",
        lambda _repo: (pn_submit.OFFICIAL_REPOSITORY, "WRITE", "main"),
    )
    monkeypatch.setattr(pn_submit, "verified_origin_push_url", lambda _repo: "origin")

    with pytest.raises(pn_submit.CommandError, match="candidate patch is empty"):
        pn_submit.build_submission_plan(repository, LAB, None)

    assert git(repository, "status", "--short") == status_before
    assert git(repository, "write-tree") == index_before


def test_submission_plan_rejects_check_all_and_builds_safe_default_message(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    stub_submission_dependencies(monkeypatch)

    with pytest.raises(pn_submit.CommandError, match="CheckAll"):
        pn_submit.build_submission_plan(tmp_path, LAB, "perf: try [CheckAll]")

    plan = pn_submit.build_submission_plan(tmp_path, LAB, None)
    assert plan.message == "perf: optimize data packing"
    assert plan.relative_lab == LAB
    assert plan.outgoing_commits == ("1234567 setup",)


def make_plan(repo: Path) -> pn_submit.SubmissionPlan:
    return pn_submit.SubmissionPlan(
        repo=repo,
        relative_lab=LAB,
        branch="jerefigo",
        repository=pn_submit.OFFICIAL_REPOSITORY,
        permission="WRITE",
        message="perf: optimize data packing",
        changes=(" M labs/memory_bound/data_packing/solution.h",),
        outgoing_commits=("1234567 setup",),
        has_upstream=False,
        head_commit=SHA,
        fingerprint=TREE,
        patch="diff --git a/solution.h b/solution.h",
        origin_push_url="https://github.com/dendibakh/perf-ninja.git",
    )


def submit_args(**overrides: object) -> SimpleNamespace:
    values = {
        "message": None,
        "dry_run": False,
        "yes": False,
        "no_watch": False,
        "ci_timeout": 900,
        "ci_interval": 5,
    }
    values.update(overrides)
    return SimpleNamespace(**values)


def test_submit_dry_run_checks_locally_without_mutating_repository(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    plan = make_plan(tmp_path)
    compare = Mock()
    mutate = Mock(side_effect=AssertionError("must not mutate"))
    watch = Mock(side_effect=AssertionError("must not watch"))
    monkeypatch.setattr(pn_submit, "build_submission_plan", lambda *_: plan)
    monkeypatch.setattr(pn_submit, "submit_changes", mutate)
    monkeypatch.setattr(pn_ci, "check_ci", watch)

    pn_submit.command_submit(tmp_path, LAB, submit_args(dry_run=True), compare=compare)

    compare.assert_called_once_with()
    mutate.assert_not_called()
    watch.assert_not_called()


def test_submit_aborts_without_mutation_when_confirmation_is_declined(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    plan = make_plan(tmp_path)
    compare = Mock()
    mutate = Mock(side_effect=AssertionError("must not mutate"))
    monkeypatch.setattr(pn_submit, "build_submission_plan", lambda *_: plan)
    monkeypatch.setattr(pn_submit, "confirm_submission", lambda: False)
    monkeypatch.setattr(pn_submit, "submit_changes", mutate)
    monkeypatch.setattr(pn_submit.sys.stdin, "isatty", lambda: True)

    pn_submit.command_submit(tmp_path, LAB, submit_args(), compare=compare)

    compare.assert_called_once_with()
    mutate.assert_not_called()


def test_submit_noninteractive_mode_requires_yes(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    plan = make_plan(tmp_path)
    monkeypatch.setattr(pn_submit, "build_submission_plan", lambda *_: plan)
    monkeypatch.setattr(pn_submit.sys.stdin, "isatty", lambda: False)

    with pytest.raises(pn_submit.CommandError, match="--yes"):
        pn_submit.command_submit(tmp_path, LAB, submit_args(), compare=Mock())


def test_submit_pushes_exact_commit_then_watches_exact_branch(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    plan = make_plan(tmp_path)
    compare = Mock()
    mutate = Mock(return_value=SHA)
    watch = Mock()
    monkeypatch.setattr(pn_submit, "build_submission_plan", lambda *_: plan)
    monkeypatch.setattr(pn_submit, "submit_changes", mutate)
    monkeypatch.setattr(pn_ci, "check_ci", watch)

    pn_submit.command_submit(tmp_path, LAB, submit_args(yes=True), compare=compare)

    compare.assert_called_once_with()
    mutate.assert_called_once_with(plan)
    watch.assert_called_once_with(
        tmp_path,
        SHA,
        branch="jerefigo",
        watch=True,
        timeout_seconds=900,
        interval_seconds=5,
    )


def test_submit_aborts_when_lab_state_drifts_during_local_checks(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    plan = make_plan(tmp_path)
    changed = replace(plan, fingerprint="c" * 40)
    plans = Mock(side_effect=[plan, changed])
    mutate = Mock(side_effect=AssertionError("must not mutate stale state"))
    monkeypatch.setattr(pn_submit, "build_submission_plan", plans)
    monkeypatch.setattr(pn_submit, "submit_changes", mutate)

    with pytest.raises(pn_submit.CommandError, match="changed during"):
        pn_submit.command_submit(tmp_path, LAB, submit_args(yes=True), compare=Mock())

    mutate.assert_not_called()


def test_submit_no_watch_prints_exact_followup_command(
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    plan = make_plan(tmp_path)
    monkeypatch.setattr(pn_submit, "build_submission_plan", lambda *_: plan)
    monkeypatch.setattr(pn_submit, "submit_changes", lambda _plan: SHA)
    watch = Mock()
    monkeypatch.setattr(pn_ci, "check_ci", watch)

    pn_submit.command_submit(
        tmp_path, LAB, submit_args(yes=True, no_watch=True), compare=Mock()
    )

    watch.assert_not_called()
    output = capsys.readouterr().out
    assert f"pn ci --commit {SHA} --branch jerefigo --watch" in output


def test_submit_changes_stages_only_lab_and_never_force_pushes(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    plan = make_plan(tmp_path)
    run = Mock()
    monkeypatch.setattr(pn_submit, "run_checked", run)
    monkeypatch.setattr(pn_submit, "staged_paths", lambda _repo: (str(LAB),))
    monkeypatch.setattr(pn_submit, "current_branch", lambda _repo: plan.branch)
    monkeypatch.setattr(pn_submit, "current_head", lambda _repo: plan.head_commit)
    monkeypatch.setattr(
        pn_submit, "submission_fingerprint", lambda *_: plan.fingerprint
    )
    monkeypatch.setattr(
        pn_submit, "origin_push_url", lambda _repo: plan.origin_push_url
    )
    monkeypatch.setattr(
        pn_submit,
        "git_text",
        lambda arguments, **_: (
            SHA
            if arguments[:2] == ["rev-parse", "HEAD"]
            else plan.fingerprint
            if arguments[:2] == ["rev-parse", f"{SHA}^{{tree}}"]
            else str(LAB / "solution.h")
            if arguments[0] == "diff-tree"
            else ""
        ),
    )

    assert pn_submit.submit_changes(plan) == SHA

    commands = [call.args[0] for call in run.call_args_list]
    assert ["git", "add", "--all", "--", str(LAB)] in commands
    assert ["git", "commit", "-m", plan.message] in commands
    assert ["git", "push", "origin", f"{SHA}:refs/heads/jerefigo"] in commands
    assert [
        "git",
        "branch",
        "--set-upstream-to",
        "origin/jerefigo",
        "jerefigo",
    ] in commands
    assert not any(
        "--force" in argument for command in commands for argument in command
    )
    assert not any(command[:3] == ["git", "config", "--global"] for command in commands)


def test_submit_changes_rechecks_staged_scope_after_git_add(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    plan = make_plan(tmp_path)
    monkeypatch.setattr(pn_submit, "run_checked", Mock())
    monkeypatch.setattr(pn_submit, "current_branch", lambda _repo: plan.branch)
    monkeypatch.setattr(pn_submit, "current_head", lambda _repo: plan.head_commit)
    monkeypatch.setattr(
        pn_submit, "submission_fingerprint", lambda *_: plan.fingerprint
    )
    monkeypatch.setattr(
        pn_submit, "origin_push_url", lambda _repo: plan.origin_push_url
    )
    monkeypatch.setattr(
        pn_submit,
        "staged_paths",
        lambda _repo: (str(LAB / "solution.h"), "labs/misc/warmup/solution.cpp"),
    )

    with pytest.raises(pn_submit.CommandError, match="outside"):
        pn_submit.submit_changes(plan)


def test_submit_changes_rejects_branch_drift_before_staging(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    plan = make_plan(tmp_path)
    mutate = Mock(side_effect=AssertionError("must not stage"))
    monkeypatch.setattr(pn_submit, "run_checked", mutate)
    monkeypatch.setattr(pn_submit, "current_branch", lambda _repo: "other-branch")

    with pytest.raises(pn_submit.CommandError, match="branch changed"):
        pn_submit.submit_changes(plan)

    mutate.assert_not_called()


def test_name_status_paths_include_both_sides_of_rename() -> None:
    output = (
        "R100\0labs/misc/warmup/old.cpp\0"
        "labs/memory_bound/data_packing/new.cpp\0"
        "M\0labs/memory_bound/data_packing/solution.h\0"
    )

    assert pn_submit.name_status_paths(output) == (
        "labs/misc/warmup/old.cpp",
        "labs/memory_bound/data_packing/new.cpp",
        "labs/memory_bound/data_packing/solution.h",
    )


@pytest.mark.parametrize(
    "url",
    [
        "https://github.com/dendibakh/perf-ninja.git",
        "git@github.com:dendibakh/perf-ninja.git",
        "ssh://git@github.com/dendibakh/perf-ninja.git",
    ],
)
def test_verified_origin_push_url_accepts_only_official_github_remote(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, url: str
) -> None:
    monkeypatch.setattr(pn_submit, "origin_push_urls", lambda _repo: (url,))

    assert pn_submit.verified_origin_push_url(tmp_path) == url


def test_verified_origin_push_url_rejects_other_or_multiple_destinations(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(
        pn_submit,
        "origin_push_urls",
        lambda _repo: ("https://github.com/jerecoder/perf-ninja.git",),
    )
    with pytest.raises(pn_submit.CommandError, match="official repository"):
        pn_submit.verified_origin_push_url(tmp_path)

    monkeypatch.setattr(
        pn_submit,
        "origin_push_urls",
        lambda _repo: (
            "https://github.com/dendibakh/perf-ninja.git",
            "ssh://mirror.invalid/perf-ninja.git",
        ),
    )
    with pytest.raises(pn_submit.CommandError, match="exactly one"):
        pn_submit.verified_origin_push_url(tmp_path)


def test_outgoing_commits_only_accepts_current_origin_branch_as_upstream(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    git_text = Mock(return_value="1234567 outgoing")
    monkeypatch.setattr(pn_submit, "git_text", git_text)

    def optional(arguments: list[str], **_kwargs: object) -> str:
        if "@{upstream}" in arguments:
            return "origin/main"
        if arguments[:2] == ["rev-parse", "--verify"]:
            return SHA
        return ""

    monkeypatch.setattr(pn_submit, "optional_git_text", optional)

    commits, has_upstream = pn_submit.outgoing_commits(tmp_path, "main", "jerefigo")

    assert commits == ("1234567 outgoing",)
    assert has_upstream is False
    assert git_text.call_args.args[0][-1] == "origin/jerefigo..HEAD"


def git(repository: Path, *arguments: str) -> str:
    return subprocess.run(
        ["git", *arguments],
        cwd=repository,
        text=True,
        capture_output=True,
        check=True,
    ).stdout.strip()


def test_lab_snapshot_includes_untracked_files_without_touching_real_index(
    tmp_path: Path,
) -> None:
    repository = tmp_path / "repo"
    solution = repository / LAB / "solution.h"
    extra = repository / LAB / "extra.cpp"
    solution.parent.mkdir(parents=True)
    solution.write_text("baseline\n", encoding="utf-8")
    subprocess.run(
        ["git", "init", "-b", "main", repository], check=True, capture_output=True
    )
    git(repository, "config", "user.name", "Test")
    git(repository, "config", "user.email", "test@example.invalid")
    git(repository, "add", ".")
    git(repository, "commit", "-m", "baseline")
    solution.write_text("optimized\n", encoding="utf-8")
    extra.write_text("new lab file\n", encoding="utf-8")
    original_index_tree = git(repository, "write-tree")

    fingerprint_before = pn_submit.submission_fingerprint(repository, LAB)
    patch = pn_submit.lab_patch(repository, LAB)

    assert git(repository, "write-tree") == original_index_tree
    assert str(LAB / "extra.cpp") in patch
    git(repository, "add", "--all", "--", str(LAB))
    assert pn_submit.submission_fingerprint(repository, LAB) == fingerprint_before


def test_submit_changes_real_git_commit_preserves_unrelated_worktree(
    tmp_path: Path,
) -> None:
    repository = tmp_path / "repo"
    remote = tmp_path / "remote.git"
    data_solution = repository / LAB / "solution.h"
    warmup_solution = repository / "labs/misc/warmup/solution.cpp"
    data_solution.parent.mkdir(parents=True)
    warmup_solution.parent.mkdir(parents=True)
    data_solution.write_text("baseline data\n", encoding="utf-8")
    warmup_solution.write_text("baseline warmup\n", encoding="utf-8")
    subprocess.run(
        ["git", "init", "-b", "main", repository], check=True, capture_output=True
    )
    git(repository, "config", "user.name", "Test")
    git(repository, "config", "user.email", "test@example.invalid")
    git(repository, "add", ".")
    git(repository, "commit", "-m", "baseline")
    git(repository, "switch", "-c", "jerefigo")
    subprocess.run(["git", "init", "--bare", remote], check=True, capture_output=True)
    git(repository, "remote", "add", "origin", str(remote))
    data_solution.write_text("optimized data\n", encoding="utf-8")
    extra_lab_file = repository / LAB / "extra.cpp"
    extra_lab_file.write_text("new lab file\n", encoding="utf-8")
    warmup_solution.write_text("user warmup\n", encoding="utf-8")
    (repository / "llvm.sh").write_text("user script\n", encoding="utf-8")
    head = git(repository, "rev-parse", "HEAD")
    plan = pn_submit.SubmissionPlan(
        repo=repository,
        relative_lab=LAB,
        branch="jerefigo",
        repository=pn_submit.OFFICIAL_REPOSITORY,
        permission="WRITE",
        message="perf: optimize data packing",
        changes=(
            " M labs/memory_bound/data_packing/solution.h",
            "?? labs/memory_bound/data_packing/extra.cpp",
        ),
        outgoing_commits=(),
        has_upstream=False,
        head_commit=head,
        fingerprint=pn_submit.submission_fingerprint(repository, LAB),
        patch=pn_submit.lab_patch(repository, LAB),
        origin_push_url=str(remote),
    )

    commit = pn_submit.submit_changes(plan)

    assert git(repository, "show", "--format=", "--name-only", commit).splitlines() == [
        str(LAB / "extra.cpp"),
        str(LAB / "solution.h"),
    ]
    assert git(repository, "status", "--short") == (
        "M labs/misc/warmup/solution.cpp\n?? llvm.sh"
    )
    assert warmup_solution.read_text(encoding="utf-8") == "user warmup\n"
    assert (repository / "llvm.sh").read_text(encoding="utf-8") == "user script\n"
    assert (
        subprocess.run(
            ["git", "--git-dir", remote, "rev-parse", "refs/heads/jerefigo"],
            text=True,
            capture_output=True,
            check=True,
        ).stdout.strip()
        == commit
    )
    assert (
        git(
            repository,
            "rev-parse",
            "--abbrev-ref",
            "--symbolic-full-name",
            "@{upstream}",
        )
        == "origin/jerefigo"
    )


def test_submit_changes_refuses_to_push_commit_changed_by_hook(tmp_path: Path) -> None:
    repository = tmp_path / "repo"
    remote = tmp_path / "remote.git"
    solution = repository / LAB / "solution.h"
    solution.parent.mkdir(parents=True)
    solution.write_text("baseline\n", encoding="utf-8")
    subprocess.run(
        ["git", "init", "-b", "main", repository], check=True, capture_output=True
    )
    git(repository, "config", "user.name", "Test")
    git(repository, "config", "user.email", "test@example.invalid")
    git(repository, "add", ".")
    git(repository, "commit", "-m", "baseline")
    git(repository, "switch", "-c", "jerefigo")
    subprocess.run(["git", "init", "--bare", remote], check=True, capture_output=True)
    git(repository, "remote", "add", "origin", str(remote))
    solution.write_text("previewed optimization\n", encoding="utf-8")
    hook = repository / ".git/hooks/pre-commit"
    hook.write_text(
        "#!/bin/sh\nprintf 'hook rewrite\\n' > "
        "labs/memory_bound/data_packing/solution.h\n"
        "git add -- labs/memory_bound/data_packing/solution.h\n",
        encoding="utf-8",
    )
    hook.chmod(0o755)
    head = git(repository, "rev-parse", "HEAD")
    plan = pn_submit.SubmissionPlan(
        repo=repository,
        relative_lab=LAB,
        branch="jerefigo",
        repository=pn_submit.OFFICIAL_REPOSITORY,
        permission="WRITE",
        message="perf: optimize data packing",
        changes=(" M labs/memory_bound/data_packing/solution.h",),
        outgoing_commits=(),
        has_upstream=False,
        head_commit=head,
        fingerprint=pn_submit.submission_fingerprint(repository, LAB),
        patch=pn_submit.lab_patch(repository, LAB),
        origin_push_url=str(remote),
    )

    with pytest.raises(pn_submit.CommandError, match="does not match.*preview"):
        pn_submit.submit_changes(plan)

    remote_ref = subprocess.run(
        ["git", "--git-dir", remote, "rev-parse", "--verify", "refs/heads/jerefigo"],
        text=True,
        capture_output=True,
        check=False,
    )
    assert remote_ref.returncode != 0


def ci_payload(
    *,
    status: str = "completed",
    conclusion: str = "success",
    sha: str = SHA,
    branch: str = "jerefigo",
) -> list[dict[str, object]]:
    return [
        {
            "databaseId": index,
            "workflowName": workflow,
            "status": status,
            "conclusion": conclusion,
            "url": f"https://example.invalid/{index}",
            "attempt": 1,
            "createdAt": f"2026-07-16T00:00:0{index}Z",
            "headSha": sha,
            "headBranch": branch,
            "event": "push",
        }
        for index, workflow in enumerate(pn_ci.EXPECTED_WORKFLOWS, start=1)
    ]


def test_parse_ci_runs_ignores_other_workflows_and_selects_newest_attempt() -> None:
    payload = ci_payload()
    payload.append(
        {
            **payload[0],
            "databaseId": 99,
            "attempt": 2,
            "createdAt": "2026-07-16T01:00:00Z",
            "status": "in_progress",
            "conclusion": "",
        }
    )
    payload.append(
        {
            **payload[0],
            "databaseId": 100,
            "workflowName": "Copilot code review",
        }
    )

    runs = pn_ci.parse_ci_runs(payload, SHA, "jerefigo")

    assert set(runs) == set(pn_ci.EXPECTED_WORKFLOWS)
    assert runs[pn_ci.EXPECTED_WORKFLOWS[0]].database_id == 99


def test_check_ci_reports_success_for_all_three_workflows(
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    monkeypatch.setattr(
        pn_ci,
        "github_repository_access",
        lambda _repo: (pn_submit.OFFICIAL_REPOSITORY, "WRITE", "main"),
    )
    monkeypatch.setattr(pn_ci, "resolve_commit", lambda *_: SHA)
    monkeypatch.setattr(
        pn_ci,
        "list_ci_runs",
        lambda *_args, **_kwargs: pn_ci.parse_ci_runs(ci_payload(), SHA, "jerefigo"),
    )

    pn_ci.check_ci(
        tmp_path,
        "HEAD",
        branch="jerefigo",
        watch=False,
        timeout_seconds=900,
        interval_seconds=5,
    )

    assert "Official CI passed" in capsys.readouterr().out


def test_check_ci_nonwatch_returns_pending_for_missing_or_running_workflows(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(
        pn_ci,
        "github_repository_access",
        lambda _repo: (pn_submit.OFFICIAL_REPOSITORY, "WRITE", "main"),
    )
    monkeypatch.setattr(pn_ci, "resolve_commit", lambda *_: SHA)
    running = pn_ci.parse_ci_runs(
        ci_payload(status="queued", conclusion=""), SHA, "jerefigo"
    )
    monkeypatch.setattr(pn_ci, "list_ci_runs", lambda *_args, **_kwargs: running)

    with pytest.raises(pn_ci.CiPending):
        pn_ci.check_ci(
            tmp_path,
            "HEAD",
            branch="jerefigo",
            watch=False,
            timeout_seconds=900,
            interval_seconds=5,
        )


def test_check_ci_watch_tolerates_delayed_registration(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(
        pn_ci,
        "github_repository_access",
        lambda _repo: (pn_submit.OFFICIAL_REPOSITORY, "WRITE", "main"),
    )
    monkeypatch.setattr(pn_ci, "resolve_commit", lambda *_: SHA)
    successful = pn_ci.parse_ci_runs(ci_payload(), SHA, "jerefigo")
    list_runs = Mock(side_effect=[{}, successful])
    sleep = Mock()
    monkeypatch.setattr(pn_ci, "list_ci_runs", list_runs)
    monkeypatch.setattr(pn_ci.time, "sleep", sleep)
    monkeypatch.setattr(pn_ci.time, "monotonic", lambda: 0.0)

    pn_ci.check_ci(
        tmp_path,
        "HEAD",
        branch="jerefigo",
        watch=True,
        timeout_seconds=900,
        interval_seconds=5,
    )

    assert list_runs.call_count == 2
    sleep.assert_called_once_with(5)


def test_check_ci_failure_fetches_failed_logs_and_returns_failure(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(
        pn_ci,
        "github_repository_access",
        lambda _repo: (pn_submit.OFFICIAL_REPOSITORY, "WRITE", "main"),
    )
    monkeypatch.setattr(pn_ci, "resolve_commit", lambda *_: SHA)
    payload = ci_payload()
    payload[1]["conclusion"] = "failure"
    failed = pn_ci.parse_ci_runs(payload, SHA, "jerefigo")
    monkeypatch.setattr(pn_ci, "list_ci_runs", lambda *_args, **_kwargs: failed)
    logs = Mock()
    monkeypatch.setattr(pn_ci, "show_failed_logs", logs)

    with pytest.raises(pn_ci.CiFailed):
        pn_ci.check_ci(
            tmp_path,
            "HEAD",
            branch="jerefigo",
            watch=False,
            timeout_seconds=900,
            interval_seconds=5,
        )

    logs.assert_called_once()


def test_list_ci_runs_queries_exact_commit_branch_and_push_event(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    gh_json = Mock(return_value=ci_payload())
    monkeypatch.setattr(pn_ci, "gh_json", gh_json)

    runs = pn_ci.list_ci_runs(tmp_path, pn_submit.OFFICIAL_REPOSITORY, SHA, "jerefigo")

    command = gh_json.call_args.args[0]
    assert command[:3] == ["gh", "run", "list"]
    assert ["--commit", SHA] == command[command.index("--commit") :][:2]
    assert ["--branch", "jerefigo"] == command[command.index("--branch") :][:2]
    assert ["--event", "push"] == command[command.index("--event") :][:2]
    assert len(runs) == 3
