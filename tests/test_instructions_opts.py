from utils import LOC_TEST_DATA, assert_command_failure

DUMMY_FILE = str(LOC_TEST_DATA / "dummy_basic.py")


def test_empty_instructions_file_arg() -> None:
    stderr = assert_command_failure(DUMMY_FILE, "--file=")
    assert "Instructions filename was not provided. Cannot proceed" in stderr


def test_missing_instructions_file() -> None:
    stderr = assert_command_failure(DUMMY_FILE, "--file=foo.txt")
    assert "File 'foo.txt' does not exist!" in stderr


def test_empty_instructions_file() -> None:
    stderr = assert_command_failure(
        DUMMY_FILE, f'--file={LOC_TEST_DATA / "edit_empty.txt"}'
    )
    assert "Instructions are empty!" in stderr


def test_empty_instructions() -> None:
    stderr = assert_command_failure(DUMMY_FILE, "--instructions=")
    assert "CLI instructions are empty. Cannot proceed" in stderr
