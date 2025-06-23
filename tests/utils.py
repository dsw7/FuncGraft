from functools import cache
from os import environ


@cache
def get_gpe_binary() -> str:
    return environ["PATH_BIN"]
