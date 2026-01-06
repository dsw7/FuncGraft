from datetime import datetime
from pytest import mark
from utils import assert_command_success, assert_command_failure, LOC_TEST_DATA


@mark.test_misc
@mark.parametrize("option", ["-h", "--help"])
def test_help(option: str) -> None:
    assert_command_success(option)


@mark.test_misc
def test_no_opts_or_args() -> None:
    assert_command_success()


@mark.test_misc
def test_unknown_opt() -> None:
    stderr = assert_command_failure("-X")
    assert "Unknown option passed to command" in stderr


@mark.test_misc
def test_copyright() -> None:
    stdout = assert_command_success("--help")
    assert (
        f"-- FuncGraft | Copyright (C) {datetime.now().year} by David Weber" in stdout
    )


# misc. prompting tests
DUMMY_FILE = str(LOC_TEST_DATA / "dummy_basic.py")


@mark.test_misc
def test_empty_instructions_file_arg() -> None:
    stderr = assert_command_failure(DUMMY_FILE, "--file=")
    assert "Instructions filename was not provided. Cannot proceed" in stderr


@mark.test_misc
def test_missing_instructions_file() -> None:
    stderr = assert_command_failure(DUMMY_FILE, "--file=foo.txt")
    assert "File 'foo.txt' does not exist!" in stderr


@mark.test_misc
def test_empty_instructions_file() -> None:
    stderr = assert_command_failure(
        DUMMY_FILE, f'--file={LOC_TEST_DATA / "edit_empty.txt"}'
    )
    assert "Instructions are empty!" in stderr


@mark.test_misc
def test_empty_instructions() -> None:
    stderr = assert_command_failure(DUMMY_FILE, "--instructions=")
    assert "CLI instructions are empty. Cannot proceed" in stderr
