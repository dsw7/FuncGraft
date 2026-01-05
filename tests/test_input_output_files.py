from .utils import LOC_TEST_DATA, assert_command_failure


def test_input_file_is_empty() -> None:
    stderr = assert_command_failure("")
    assert "No filename was provided. Cannot proceed" in stderr


def test_input_file_is_not_a_file() -> None:
    stderr = assert_command_failure("/tmp", "-iFoo")
    assert "Input '/tmp' is not a file!" in stderr


def test_input_file_is_missing() -> None:
    stderr = assert_command_failure("abc.txt", "-iFoo")
    assert "File 'abc.txt' does not exist!" in stderr


def test_output_file_is_empty() -> None:
    path_file = str(LOC_TEST_DATA / "dummy_basic.py")
    stderr = assert_command_failure(path_file, "--output=")
    assert "Output filename was not provided. Cannot proceed" in stderr
