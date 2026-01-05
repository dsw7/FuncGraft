from datetime import datetime
from .utils import ExtendedTestCase


class TestMisc(ExtendedTestCase):

    def test_help(self) -> None:
        for option in ["-h", "--help"]:
            with self.subTest(option=option):
                self.assertSuccess(option)

    def test_no_opts_or_args(self) -> None:
        self.assertSuccess()

    def test_unknown_opt(self) -> None:
        stderr = self.assertFailure("-X")
        self.assertIn("Unknown option passed to command", stderr)

    def test_copyright(self) -> None:
        stdout = self.assertSuccess("--help")
        self.assertIn(
            f"-- FuncGraft | Copyright (C) {datetime.now().year} by David Weber", stdout
        )
