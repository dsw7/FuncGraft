from pathlib import Path
from typing import Generator
from pytest import mark, fixture
from utils import assert_command_success, assert_python_script_runs


@fixture
def file_to_edit() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/file_to_edit.py")
    path_to_file.write_text(
        """
def main() -> None:
    nums = [1, c, 3]
    print(f'The sum is {sum(nums)}')

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


@fixture
def file_to_edit_with_delims() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/file_to_edit.py")
    path_to_file.write_text(
        """
@@@
def get_sum() -> int:
    return 10 + c
@@@

def main() -> None:
    c = 1
    nums = [c, get_sum(), 3]
    print(f'The sum is {sum(nums)}')

if __name__ == '__main__':
    main()
""",
        encoding="utf-8",
    )
    yield path_to_file

    try:
        path_to_file.unlink()
    except FileNotFoundError:
        pass


@fixture
def instructions_file() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/instructions.txt")
    path_to_file.write_text(
        "Replace the variable `c` with the integer 10", encoding="utf-8"
    )

    yield path_to_file

    try:
        path_to_file.unlink()
    except FileNotFoundError:
        pass


@fixture
def unknown_file() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/foobar.abc")
    path_to_file.write_text("foobar", encoding="utf-8")

    yield path_to_file

    try:
        path_to_file.unlink()
    except FileNotFoundError:
        pass


@mark.parametrize("provider", ["ollama", "openai"])
def test_read_instructions_from_file(
    provider: str, file_to_edit: Path, instructions_file: Path
) -> None:
    assert_command_success(
        f"{file_to_edit}",
        f"--provider={provider}",
        f"--file={instructions_file}",
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


@mark.parametrize("provider", ["ollama", "openai"])
def test_print_prompt_with_verbose_flag(provider: str, file_to_edit: Path) -> None:
    instructions = "Replace the variable `c` with the integer 5"
    stdout = assert_command_success(
        f"{file_to_edit}",
        f"--provider={provider}",
        f"--instructions='{instructions}'",
        f"--output={file_to_edit}",
        "--verbose",
    )
    assert "Prompt:" in stdout
    assert "The sum is 9" in assert_python_script_runs(file_to_edit)


@mark.parametrize("provider", ["ollama", "openai"])
def test_only_edit_between_delims(
    provider: str, file_to_edit_with_delims: Path
) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    assert_command_success(
        f"{file_to_edit_with_delims}",
        f"--provider={provider}",
        f"--instructions='{instructions}'",
        f"--output={file_to_edit_with_delims}",
    )
    assert "The sum is 17" in assert_python_script_runs(file_to_edit_with_delims)


@mark.parametrize("provider", ["ollama", "openai"])
def test_work_on_unknown_file(provider: str, unknown_file: Path) -> None:
    instructions = "Capitalize all words in the file"
    assert_command_success(
        f"{unknown_file}",
        f"--provider={provider}",
        f"--instructions='{instructions}'",
        f"--output={unknown_file}",
    )
    assert "Foobar" in unknown_file.read_text(encoding="utf-8")
