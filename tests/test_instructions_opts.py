from .utils import LOC_TEST_DATA, ExtendedTestCase


class TestInstructionsOpts(ExtendedTestCase):
    dummy_file = str(LOC_TEST_DATA / "dummy_basic.py")

    def test_empty_instructions_file_arg(self) -> None:
        stderr = self.assertFailure(self.dummy_file, "--file=")
        self.assertIn("Instructions filename was not provided. Cannot proceed", stderr)

    def test_missing_instructions_file(self) -> None:
        stderr = self.assertFailure(self.dummy_file, "--file=foo.txt")
        self.assertIn("File 'foo.txt' does not exist!", stderr)

    def test_empty_instructions_file(self) -> None:
        stderr = self.assertFailure(
            self.dummy_file, f'--file={LOC_TEST_DATA / "edit_empty.txt"}'
        )
        self.assertIn("Instructions are empty!", stderr)

    def test_empty_instructions(self) -> None:
        stderr = self.assertFailure(self.dummy_file, "--instructions=")
        self.assertIn("CLI instructions are empty. Cannot proceed", stderr)
