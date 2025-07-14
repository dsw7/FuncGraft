from os import remove
from subprocess import run, PIPE
from unittest import TestCase
from .utils import get_gpe_binary, LOC_TEST_DATA

OUTPUT_PATH = "/tmp/dummy_basic.py"


class TestEditing(TestCase):

    def tearDown(self) -> None:
        try:
            remove(OUTPUT_PATH)
        except FileNotFoundError:
            pass

    def test_read_instructions_from_file(self) -> None:
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_basic.py",
            f'--file={LOC_TEST_DATA / "edit.txt"}',
            f"-o{OUTPUT_PATH}",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)

        process = run(["python3", OUTPUT_PATH], stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertIn("The sum is 14", process.stdout)

    def test_read_instructions_from_cli(self) -> None:
        instructions = "Replace the variable `c` with the integer 5"
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_basic.py",
            f"--instructions='{instructions}'",
            f"-o{OUTPUT_PATH}",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)

        process = run(["python3", OUTPUT_PATH], stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertIn("The sum is 9", process.stdout)

    def test_only_edit_between_delims(self) -> None:
        instructions = "Replace the variable `c` with the integer 3"
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_with_delims.py",
            f"--instructions='{instructions}'",
            f"-o{OUTPUT_PATH}",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)

        process = run(["python3", OUTPUT_PATH], stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertIn("The sum is 17", process.stdout)

    def test_bad_delim_placement(self) -> None:
        instructions = "Replace the variable `c` with the integer 3"
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_with_bad_delims.py",
            f"--instructions='{instructions}'",
            f"-o{OUTPUT_PATH}",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("No matching closing delimiter line", process.stderr)

    def test_work_on_empty_file(self) -> None:
        instructions = "Replace the variable `c` with the integer 3"
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_empty.py",
            f"--instructions='{instructions}'",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1, process.stdout)
        self.assertIn("Body is empty. Cannot extract code block", process.stderr)
