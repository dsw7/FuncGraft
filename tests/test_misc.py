from datetime import datetime
from subprocess import run, PIPE
from unittest import TestCase
from .utils import get_gpe_binary


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

    def test_unknown_opt(self) -> None:
        command = [get_gpe_binary(), "-X"]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("Unknown option passed to command", process.stderr)

    #    def test_missing_instructions(self) -> None:
    #        command = [get_gpe_binary(), LOC_TEST_DATA / "dummy_basic.py"]
    #        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
    #        self.assertEqual(process.returncode, 1)
    #        self.assertIn(
    #            "No instructions provided by file or command line", process.stderr
    #        )

    def test_copyright(self) -> None:
        proc = self.assertSuccess("--help")
        self.assertIn(
            f"-- FuncGraft | Copyright (C) {datetime.now().year} by David Weber",
            proc.stdout.strip(),
        )
