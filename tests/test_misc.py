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
