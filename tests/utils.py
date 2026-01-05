from os import environ
from pathlib import Path
from subprocess import run, PIPE

LOC_TEST_DATA = Path("tests/test_files")


def assert_command_success(*args: str) -> str:
    command = [environ["PATH_BIN"]]
    command.extend(args)
    process = run(command, stdout=PIPE, stderr=PIPE, text=True)
    assert process.returncode == 0, process.stderr
    return process.stdout


def assert_command_failure(*args: str) -> str:
    command = [environ["PATH_BIN"]]
    command.extend(args)
    process = run(command, stdout=PIPE, stderr=PIPE, text=True)
    assert process.returncode == 1
    return process.stderr
