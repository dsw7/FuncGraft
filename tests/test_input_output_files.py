from subprocess import run, PIPE
from unittest import TestCase
from .utils import get_gpe_binary, LOC_TEST_DATA


class TestInputFileOptions(TestCase):
    def test_input_file_is_empty(self) -> None:
        command = [get_gpe_binary(), ""]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("No filename was provided. Cannot proceed", process.stderr)

    def test_input_file_is_not_a_file(self) -> None:
        command = [get_gpe_binary(), "/tmp", "-iFoo"]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("Input '/tmp' is not a file!", process.stderr)

    def test_input_file_is_missing(self) -> None:
        command = [get_gpe_binary(), "abc.txt", "-iFoo"]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("File 'abc.txt' does not exist!", process.stderr)


class TestOutputFileOptions(TestCase):
    def test_output_file_is_empty(self) -> None:
        command = [get_gpe_binary(), LOC_TEST_DATA / "dummy_basic.py", "--output="]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn(
            "Output filename was not provided. Cannot proceed", process.stderr
        )
