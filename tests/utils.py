from functools import cache
from os import environ
from pathlib import Path

LOC_TEST_DATA = Path("tests/test_files")


@cache
def get_gpe_binary() -> str:
    return environ["PATH_BIN"]
