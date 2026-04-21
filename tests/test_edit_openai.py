from pytest import mark
from utils import LOC_TEST_DATA, assert_command_success, assert_python_script_runs


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
