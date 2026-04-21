from datetime import datetime
from pathlib import Path
from typing import Generator
from pytest import mark, fixture
from utils import assert_command_success, assert_command_failure, LOC_TEST_DATA


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


def test_invalid_provider() -> None:
    stderr = assert_command_failure("--provider=foobar", "foobar.py")
    assert "Invalid LLM provider" in stderr


# misc. prompting tests
DUMMY_FILE = str(LOC_TEST_DATA / "dummy_basic.py")


@fixture
def empty_instructions_file(text_file: Path) -> Generator[Path, None, None]:
    text_file.write_text("", encoding="utf-8")
    yield text_file


def test_empty_instructions_file_arg() -> None:
    stderr = assert_command_failure(DUMMY_FILE, "--file=")
    assert "Instructions filename was not provided. Cannot proceed" in stderr


def test_missing_instructions_file() -> None:
    stderr = assert_command_failure(DUMMY_FILE, "--file=foo.txt")
    assert "File 'foo.txt' does not exist!" in stderr


def test_empty_instructions_file(empty_instructions_file: Path) -> None:
    stderr = assert_command_failure(DUMMY_FILE, f"--file={empty_instructions_file}")
    assert "Instructions are empty!" in stderr


def test_empty_instructions() -> None:
    stderr = assert_command_failure(DUMMY_FILE, "--instructions=")
    assert "CLI instructions are empty. Cannot proceed" in stderr
