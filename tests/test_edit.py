from os import remove
from subprocess import run, PIPE
from .utils import LOC_TEST_DATA, assert_command_success, assert_command_failure

OUTPUT_PATH = "/tmp/dummy_basic.py"


def tearDown() -> None:
    try:
        remove(OUTPUT_PATH)
    except FileNotFoundError:
        pass


def test_read_instructions_from_file() -> None:
    assert_command_success(
        str(LOC_TEST_DATA / "dummy_basic.py"),
        f'--file={LOC_TEST_DATA / "edit.txt"}',
        f"-o{OUTPUT_PATH}",
    )
    process = run(["python3", OUTPUT_PATH], stdout=PIPE, stderr=PIPE, text=True)
    assert process.returncode == 0, process.stderr
    assert "The sum is 14" in process.stdout


def test_read_instructions_from_cli() -> None:
    instructions = "Replace the variable `c` with the integer 5"
    stdout = assert_command_success(
        str(LOC_TEST_DATA / "dummy_basic.py"),
        f"--instructions='{instructions}'",
        f"-o{OUTPUT_PATH}",
    )
    assert "Prompt:" not in stdout  # Prompt should not print by default

    process = run(["python3", OUTPUT_PATH], stdout=PIPE, stderr=PIPE, text=True)
    assert process.returncode == 0, process.stderr
    assert "The sum is 9" in process.stdout


def test_print_prompt_with_verbose_flag() -> None:
    instructions = "Replace the variable `c` with the integer 5"
    stdout = assert_command_success(
        str(LOC_TEST_DATA / "dummy_basic.py"),
        f"--instructions='{instructions}'",
        f"-o{OUTPUT_PATH}",
        "--verbose",
    )
    assert "Prompt:" in stdout

    process = run(["python3", OUTPUT_PATH], stdout=PIPE, stderr=PIPE, text=True)
    assert process.returncode == 0, process.stderr
    assert "The sum is 9" in process.stdout


def test_only_edit_between_delims() -> None:
    instructions = "Replace the variable `c` with the integer 3"
    assert_command_success(
        str(LOC_TEST_DATA / "dummy_with_delims.py"),
        f"--instructions='{instructions}'",
        f"-o{OUTPUT_PATH}",
    )
    process = run(["python3", OUTPUT_PATH], stdout=PIPE, stderr=PIPE, text=True)
    assert process.returncode == 0, process.stderr
    assert "The sum is 17" in process.stdout


def test_bad_delim_placement() -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        str(LOC_TEST_DATA / "dummy_with_bad_delims.py"),
        f"--instructions='{instructions}'",
        f"-o{OUTPUT_PATH}",
    )
    assert "No matching closing delimiter line" in stderr


def test_bad_delim_placement_2() -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        str(LOC_TEST_DATA / "dummy_with_bad_delims_2.py"),
        f"--instructions='{instructions}'",
        f"-o{OUTPUT_PATH}",
    )
    assert "The number of delimiter lines must be exactly 2" in stderr


def test_work_on_empty_file() -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        str(LOC_TEST_DATA / "dummy_empty.py"), f"--instructions='{instructions}'"
    )
    assert "The file does not contain any code" in stderr


def test_work_with_empty_delims() -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        str(LOC_TEST_DATA / "dummy_with_empty_delims.py"),
        f"--instructions='{instructions}'",
    )
    assert "The delimited block does not contain any code" in stderr


def test_work_with_empty_delims_2() -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        str(LOC_TEST_DATA / "dummy_with_empty_delims_2.py"),
        f"--instructions='{instructions}'",
    )
    assert "The delimited block does not contain any code" in stderr


def test_work_on_unknown_file() -> None:
    instructions = "Capitalize all words in the file"
    assert_command_success(
        str(LOC_TEST_DATA / "unknown_file_type.abc"),
        f"--instructions='{instructions}'",
        f"-o{OUTPUT_PATH}",  # Will write to a .py file but it's okay
    )

    with open(OUTPUT_PATH) as f:
        assert "Foobar" in f.read()
