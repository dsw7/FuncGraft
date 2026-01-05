from .utils import LOC_TEST_DATA, ExtendedTestCase


class TestInputFileOptions(ExtendedTestCase):
    def test_input_file_is_empty(self) -> None:
        stderr = self.assertFailure("")
        self.assertIn("No filename was provided. Cannot proceed", stderr)

    def test_input_file_is_not_a_file(self) -> None:
        stderr = self.assertFailure("/tmp", "-iFoo")
        self.assertIn("Input '/tmp' is not a file!", stderr)

    def test_input_file_is_missing(self) -> None:
        stderr = self.assertFailure("abc.txt", "-iFoo")
        self.assertIn("File 'abc.txt' does not exist!", stderr)


class TestOutputFileOptions(ExtendedTestCase):
    def test_output_file_is_empty(self) -> None:
        path_file = str(LOC_TEST_DATA / "dummy_basic.py")
        stderr = self.assertFailure(path_file, "--output=")
        self.assertIn("Output filename was not provided. Cannot proceed", stderr)
