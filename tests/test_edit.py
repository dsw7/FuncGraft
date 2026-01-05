from os import remove
from subprocess import run, PIPE
from typing import Generator
from pytest import fixture
from utils import LOC_TEST_DATA, assert_command_success, assert_command_failure


@fixture
def outputted_script() -> Generator[str, None, None]:
    path_to_outputted_script = "/tmp/dummy_basic.py"
    yield path_to_outputted_script

    try:
        remove(path_to_outputted_script)
    except FileNotFoundError:
        pass


def run_outputted_python_script(python_script: str) -> str:
    # some tests will edit a python script with LLMs and place the edited copy elsewhere
    # run the edited copy to see if it works after modification by an LLM
    process = run(["python3", python_script], stdout=PIPE, stderr=PIPE, text=True)
    assert process.returncode == 0, process.stderr
    return process.stdout


def test_read_instructions_from_file(outputted_script: str) -> None:
    assert_command_success(
        str(LOC_TEST_DATA / "dummy_basic.py"),
        f'--file={LOC_TEST_DATA / "edit.txt"}',
        f"-o{outputted_script}",
    )
    assert "The sum is 14" in run_outputted_python_script(outputted_script)


def test_read_instructions_from_cli(outputted_script: str) -> None:
    instructions = "Replace the variable `c` with the integer 5"
    stdout = assert_command_success(
        str(LOC_TEST_DATA / "dummy_basic.py"),
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",
    )
    assert "Prompt:" not in stdout  # Prompt should not print by default
    assert "The sum is 9" in run_outputted_python_script(outputted_script)


def test_print_prompt_with_verbose_flag(outputted_script: str) -> None:
    instructions = "Replace the variable `c` with the integer 5"
    stdout = assert_command_success(
        str(LOC_TEST_DATA / "dummy_basic.py"),
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",
        "--verbose",
    )
    assert "Prompt:" in stdout
    assert "The sum is 9" in run_outputted_python_script(outputted_script)


def test_only_edit_between_delims(outputted_script: str) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    assert_command_success(
        str(LOC_TEST_DATA / "dummy_with_delims.py"),
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",
    )
    assert "The sum is 17" in run_outputted_python_script(outputted_script)


def test_bad_delim_placement(outputted_script: str) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        str(LOC_TEST_DATA / "dummy_with_bad_delims.py"),
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",
    )
    assert "No matching closing delimiter line" in stderr


def test_bad_delim_placement_2(outputted_script: str) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        str(LOC_TEST_DATA / "dummy_with_bad_delims_2.py"),
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",
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


def test_work_on_unknown_file(outputted_script: str) -> None:
    instructions = "Capitalize all words in the file"
    assert_command_success(
        str(LOC_TEST_DATA / "unknown_file_type.abc"),
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",  # Will write to a .py file but it's okay
    )

    with open(outputted_script) as f:
        assert "Foobar" in f.read()
