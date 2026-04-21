from pathlib import Path
from typing import Generator
from pytest import fixture


@fixture
def python_file() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/funcgraft_test_file.py")
    yield path_to_file

    try:
        path_to_file.unlink()
    except FileNotFoundError:
        pass


@fixture
def text_file() -> Generator[Path, None, None]:
    path_to_file = Path("/tmp/funcgraft_test_file.txt")
    yield path_to_file

    try:
        path_to_file.unlink()
    except FileNotFoundError:
        pass


@fixture
def dummy_python_file(python_file: Path) -> Generator[str, None, None]:
    # Useful for cases where we just need to satisfy the input file
    # requirement but we're otherwise doing nothing with the file
    python_file.write_text("print('Lorem ipsum...')", encoding="utf-8")
    yield str(python_file)
