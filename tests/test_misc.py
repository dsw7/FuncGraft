from subprocess import run, PIPE
from unittest import TestCase
from .utils import get_gpe_binary, LOC_TEST_DATA


class TestMisc(TestCase):

    def test_help(self) -> None:
        for option in ["-h", "--help"]:
            with self.subTest(option=option):
                command = [get_gpe_binary(), option]
                process = run(command, stdout=PIPE, stderr=PIPE, text=True)
                self.assertEqual(process.returncode, 0, process.stderr)

    def test_no_opts_or_args(self) -> None:
        command = [get_gpe_binary()]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)

    def test_input_file_is_not_a_file(self) -> None:
        command = [get_gpe_binary(), "/tmp", "-iFoo"]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("Input '/tmp' is not a file!", process.stderr)

    def test_missing_input_file(self) -> None:
        command = [get_gpe_binary(), "abc.txt", "-iFoo"]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("File 'abc.txt' does not exist!", process.stderr)

    def test_missing_instructions(self) -> None:
        command = [get_gpe_binary(), LOC_TEST_DATA / "dummy.py"]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn(
            "No instructions provided by file or command line", process.stderr
        )

    def test_missing_instructions_file(self) -> None:
        command = [get_gpe_binary(), LOC_TEST_DATA / "dummy.py", "--file=foo.txt"]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("File 'foo.txt' does not exist!", process.stderr)

    def test_invalid_model(self) -> None:
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy.py",
            "--instructions=tests/edit.txt",
            "--model=foobar",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn(
            "The model `foobar` does not exist or you do not have access to it.",
            process.stderr,
        )

    def test_empty_instructions(self) -> None:
        command = [get_gpe_binary(), LOC_TEST_DATA / "dummy.py", "--instructions="]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("Instructions are empty!", process.stderr)
