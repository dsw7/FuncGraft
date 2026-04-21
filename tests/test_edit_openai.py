from pytest import mark
from utils import LOC_TEST_DATA, assert_command_success, assert_python_script_runs


@mark.test_openai
def test_print_prompt_with_verbose_flag(outputted_script: str) -> None:
    instructions = "Replace the variable `c` with the integer 5"
    stdout = assert_command_success(
        "--provider=openai",
        f"{LOC_TEST_DATA}/dummy_basic.py",
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",
        "--verbose",
    )
    assert "Prompt:" in stdout
    assert "The sum is 9" in assert_python_script_runs(outputted_script)


@mark.test_openai
def test_only_edit_between_delims(outputted_script: str) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    assert_command_success(
        "--provider=openai",
        f"{LOC_TEST_DATA}/dummy_with_delims.py",
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",
    )
    assert "The sum is 17" in assert_python_script_runs(outputted_script)


@mark.test_openai
def test_work_on_unknown_file(outputted_script: str) -> None:
    instructions = "Capitalize all words in the file"
    assert_command_success(
        "--provider=openai",
        f"{LOC_TEST_DATA}/unknown_file_type.abc",
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",  # Will write to a .py file but it's okay
    )

    with open(outputted_script, encoding="utf-8") as f:
        assert "Foobar" in f.read()
