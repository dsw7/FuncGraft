from subprocess import run, PIPE
from unittest import TestCase
from .utils import get_gpe_binary, LOC_TEST_DATA


class TestEditing(TestCase):

    def test_read_instructions_from_file(self) -> None:
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_basic.py",
            f'--file={LOC_TEST_DATA / "edit.txt"}',
            "-o/tmp/dummy_basic.py",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)

        process = run(
            ["python3", "/tmp/dummy_basic.py"], stdout=PIPE, stderr=PIPE, text=True
        )
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertIn("The sum is 14", process.stdout)

    def test_read_instructions_from_cli(self) -> None:
        instructions = "Replace the variable `c` with the integer 5"
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_basic.py",
            f"--instructions='{instructions}'",
            "-o/tmp/dummy_basic.py",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)

        process = run(
            ["python3", "/tmp/dummy_basic.py"], stdout=PIPE, stderr=PIPE, text=True
        )
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertIn("The sum is 9", process.stdout)

    def test_only_edit_between_delims(self) -> None:
        instructions = "Replace the variable `c` with the integer 3"
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_with_delims.py",
            f"--instructions='{instructions}'",
            "-o/tmp/dummy_basic.py",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)

        process = run(
            ["python3", "/tmp/dummy_basic.py"], stdout=PIPE, stderr=PIPE, text=True
        )
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertIn("The sum is 17", process.stdout)

    def test_bad_delim_placement(self) -> None:
        instructions = "Replace the variable `c` with the integer 3"
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_with_bad_delims.py",
            f"--instructions='{instructions}'",
            "-o/tmp/dummy_basic.py",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("No matching closing delimiter line", process.stderr)
