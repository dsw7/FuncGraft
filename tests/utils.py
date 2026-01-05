from functools import cache
from os import environ
from pathlib import Path
from subprocess import run, PIPE
from unittest import TestCase

LOC_TEST_DATA = Path("tests/test_files")


@cache
def get_gpe_binary() -> str:
    return environ["PATH_BIN"]


class ExtendedTestCase(TestCase):

    def assertSuccess(self, *args: str) -> str:
        command = [environ["PATH_BIN"]]
        command.extend(args)
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)
        return process.stdout

    def assertFailure(self, *args: str) -> str:
        command = [environ["PATH_BIN"]]
        command.extend(args)
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        return process.stderr
