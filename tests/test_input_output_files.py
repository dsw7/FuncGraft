from pathlib import Path
from typing import Generator
from pytest import mark, fixture
from utils import LOC_TEST_DATA, assert_command_failure, remove_file_when_done


@fixture
def file_to_edit_with_bad_delims() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/file_to_edit.py")
    path_to_file.write_text(
        """
def get_sum() -> int:
    return 10 + c
@@@

def main() -> None:
    c = 1
    nums = [c, get_sum(), 3]
    print(f"The sum is {sum(nums)}")

if __name__ == "__main__":
    main()
""",
        encoding="utf-8",
    )

    yield path_to_file
    remove_file_when_done(path_to_file)


@fixture
def file_to_edit_with_bad_delims_2() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/file_to_edit.py")
    path_to_file.write_text(
        """
def get_sum() -> int:
    return 10 + c
@@@

@@@

@@@

def main() -> None:
    c = 1
    nums = [c, get_sum(), 3]
    print(f"The sum is {sum(nums)}")

if __name__ == "__main__":
    main()
""",
        encoding="utf-8",
    )

    yield path_to_file
    remove_file_when_done(path_to_file)


@fixture
def file_to_edit_is_empty() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/file_to_edit.py")
    path_to_file.write_text("", encoding="utf-8")

    yield path_to_file
    remove_file_when_done(path_to_file)


@fixture
def file_to_edit_with_empty_delims() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/file_to_edit.py")
    path_to_file.write_text(
        """
def get_sum() -> int:
    return 10 + c

# The delimiters are next to each other so program should abort since
# the code block is empty
@@@
@@@

def main() -> None:
    c = 1
    nums = [c, get_sum(), 3]
    print(f"The sum is {sum(nums)}")
""",
        encoding="utf-8",
    )

    yield path_to_file
    remove_file_when_done(path_to_file)


@fixture
def file_to_edit_with_empty_delims_2() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/file_to_edit.py")
    path_to_file.write_text(
        """
def get_sum() -> int:
    return 10 + c

# The delimiters do not delimit any actual code so the code block is considered empty
@@@


@@@

def main() -> None:
    c = 1
    nums = [c, get_sum(), 3]
    print(f"The sum is {sum(nums)}")
""",
        encoding="utf-8",
    )

    yield path_to_file
    remove_file_when_done(path_to_file)


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


@mark.parametrize("provider", ["ollama", "openai"])
def test_bad_delim_placement(provider: str, file_to_edit_with_bad_delims: Path) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        f"{file_to_edit_with_bad_delims}",
        f"--provider={provider}",
        f"--instructions='{instructions}'",
        f"-o{file_to_edit_with_bad_delims}",
    )
    assert "No matching closing delimiter line" in stderr


@mark.parametrize("provider", ["ollama", "openai"])
def test_bad_delim_placement_2(
    provider: str, file_to_edit_with_bad_delims_2: Path
) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        f"{file_to_edit_with_bad_delims_2}",
        f"--provider={provider}",
        f"--instructions='{instructions}'",
        f"-o{file_to_edit_with_bad_delims_2}",
    )
    assert "The number of delimiter lines must be exactly 2" in stderr


@mark.parametrize("provider", ["ollama", "openai"])
def test_work_on_empty_file(provider: str, file_to_edit_is_empty: Path) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        f"{file_to_edit_is_empty}",
        f"--provider={provider}",
        f"--instructions='{instructions}'",
    )
    assert "The file does not contain any code" in stderr


@mark.parametrize("provider", ["ollama", "openai"])
def test_work_with_empty_delims(
    provider: str, file_to_edit_with_empty_delims: Path
) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        f"{file_to_edit_with_empty_delims}",
        f"--provider={provider}",
        f"--instructions='{instructions}'",
    )
    assert "The delimited block does not contain any code" in stderr


@mark.parametrize("provider", ["ollama", "openai"])
def test_work_with_empty_delims_2(
    provider: str, file_to_edit_with_empty_delims_2: Path
) -> None:
    instructions = "Replace the variable `c` with the integer 3"
    stderr = assert_command_failure(
        f"{file_to_edit_with_empty_delims_2}",
        f"--provider={provider}",
        f"--instructions='{instructions}'",
    )
    assert "The delimited block does not contain any code" in stderr
