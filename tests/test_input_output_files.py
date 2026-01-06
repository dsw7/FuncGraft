from pytest import mark
from utils import LOC_TEST_DATA, assert_command_failure


@mark.test_misc
def test_input_file_is_empty() -> None:
    stderr = assert_command_failure("")
    assert "No filename was provided. Cannot proceed" in stderr


@mark.test_misc
def test_input_file_is_not_a_file() -> None:
    stderr = assert_command_failure("/tmp", "-iFoo")
    assert "Input '/tmp' is not a file!" in stderr


@mark.test_misc
def test_input_file_is_missing() -> None:
    stderr = assert_command_failure("abc.txt", "-iFoo")
    assert "File 'abc.txt' does not exist!" in stderr


@mark.test_misc
def test_output_file_is_empty() -> None:
    path_file = str(LOC_TEST_DATA / "dummy_basic.py")
    stderr = assert_command_failure(path_file, "--output=")
    assert "Output filename was not provided. Cannot proceed" in stderr


# delimiter tests


@mark.test_misc
@mark.parametrize(
    "dummy_file, errmsg",
    [
        ("dummy_with_bad_delims.py", "No matching closing delimiter line"),
        (
            "dummy_with_bad_delims_2.py",
            "The number of delimiter lines must be exactly 2",
        ),
    ],
)
def test_bad_delim_placement(
    dummy_file: str, errmsg: str, outputted_script: str
) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        f"{LOC_TEST_DATA}/{dummy_file}",
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",
    )
    assert errmsg in stderr


@mark.test_misc
def test_work_on_empty_file() -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        str(LOC_TEST_DATA / "dummy_empty.py"),
        f"--instructions='{instructions}'",
    )
    assert "The file does not contain any code" in stderr


@mark.test_misc
@mark.parametrize(
    "dummy_file", ["dummy_with_empty_delims.py", "dummy_with_empty_delims_2.py"]
)
def test_work_with_empty_delims(dummy_file: str) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        f"{LOC_TEST_DATA}/{dummy_file}", f"--instructions='{instructions}'"
    )
    assert "The delimited block does not contain any code" in stderr
