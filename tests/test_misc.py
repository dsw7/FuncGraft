from datetime import datetime
from pytest import mark
from .utils import assert_command_success, assert_command_failure


@mark.parametrize("option", ["-h", "--help"])
def test_help(option: str) -> None:
    assert_command_success(option)


def test_no_opts_or_args() -> None:
    assert_command_success()


def test_unknown_opt() -> None:
    stderr = assert_command_failure("-X")
    assert "Unknown option passed to command" in stderr


def test_copyright() -> None:
    stdout = assert_command_success("--help")
    assert (
        f"-- FuncGraft | Copyright (C) {datetime.now().year} by David Weber" in stdout
    )
