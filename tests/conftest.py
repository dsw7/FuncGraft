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
