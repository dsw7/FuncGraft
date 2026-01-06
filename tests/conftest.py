from os import remove
from typing import Generator
from pytest import fixture


@fixture
def outputted_script() -> Generator[str, None, None]:
    # some tests process an input file and emit an output file:
    # i.e. edit <input-file> -o <output-file>
    # this fixture cleans up <output-file>

    path_to_outputted_script = "/tmp/dummy_basic.py"
    yield path_to_outputted_script

    try:
        remove(path_to_outputted_script)
    except FileNotFoundError:
        pass
