from subprocess import run, PIPE
from unittest import TestCase
from .utils import get_gpe_binary, LOC_TEST_DATA


class TestInstructionsOpts(TestCase):

    def test_empty_instructions_file_arg(self) -> None:
        command = [get_gpe_binary(), LOC_TEST_DATA / "dummy_basic.py", "--file="]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn(
            "Instructions filename was not provided. Cannot proceed", process.stderr
        )

    def test_missing_instructions_file(self) -> None:
        command = [get_gpe_binary(), LOC_TEST_DATA / "dummy_basic.py", "--file=foo.txt"]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("File 'foo.txt' does not exist!", process.stderr)

    def test_empty_instructions_file(self) -> None:
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_basic.py",
            f'--file={LOC_TEST_DATA / "edit_empty.txt"}',
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("Instructions are empty!", process.stderr)

    def test_empty_instructions(self) -> None:
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_basic.py",
            "--instructions=",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("CLI instructions are empty. Cannot proceed", process.stderr)
