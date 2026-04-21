from pathlib import Path
from typing import Generator
from pytest import mark, fixture
from utils import LOC_TEST_DATA, assert_command_success, assert_python_script_runs


@fixture
def file_to_edit() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/file_to_edit.py")
    path_to_file.write_text(
        """
def main() -> None:
    nums = [1, c, 3]
    print(f"The sum is {sum(nums)}")

if __name__ == "__main__":
    main()
""",
        encoding="utf-8",
    )
    yield path_to_file

    try:
        path_to_file.unlink()
    except FileNotFoundError:
        pass


@mark.parametrize("provider", ["ollama", "openai"])
def test_read_instructions_from_file(provider: str, file_to_edit: Path) -> None:
    assert_command_success(
        f"{file_to_edit}",
        f"--provider={provider}",
        f'--file={LOC_TEST_DATA / "edit.txt"}',
        f"--output={file_to_edit}",
    )
    assert "The sum is 14" in assert_python_script_runs(file_to_edit)


@mark.parametrize("provider", ["ollama", "openai"])
def test_read_instructions_from_cli(provider: str, file_to_edit: Path) -> None:
    instructions = "Replace the variable `c` with the integer 5"
    stdout = assert_command_success(
        f"{file_to_edit}",
        f"--provider={provider}",
        f"--instructions='{instructions}'",
        f"--output={file_to_edit}",
    )
    assert "Prompt:" not in stdout  # Prompt should not print by default
    assert "The sum is 9" in assert_python_script_runs(file_to_edit)


@mark.test_ollama
def test_print_prompt_with_verbose_flag(outputted_script: str) -> None:
    instructions = "Replace the variable `c` with the integer 5"
    stdout = assert_command_success(
        "--provider=ollama",
        f"{LOC_TEST_DATA}/dummy_basic.py",
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",
        "--verbose",
    )
    assert "Prompt:" in stdout
    assert "The sum is 9" in assert_python_script_runs(outputted_script)


@mark.test_ollama
def test_only_edit_between_delims(outputted_script: str) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    assert_command_success(
        "--provider=ollama",
        f"{LOC_TEST_DATA}/dummy_with_delims.py",
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",
    )
    assert "The sum is 17" in assert_python_script_runs(outputted_script)


@mark.test_ollama
def test_work_on_unknown_file(outputted_script: str) -> None:
    instructions = "Capitalize all words in the file"
    assert_command_success(
        "--provider=ollama",
        f"{LOC_TEST_DATA}/unknown_file_type.abc",
        f"--instructions='{instructions}'",
        f"-o{outputted_script}",  # Will write to a .py file but it's okay
    )

    with open(outputted_script, encoding="utf-8") as f:
        assert "Foobar" in f.read()
