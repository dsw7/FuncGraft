from subprocess import run, PIPE
from unittest import TestCase
from .utils import get_gpe_binary, LOC_TEST_DATA


class TestModelOptions(TestCase):
    def test_empty_model(self) -> None:
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_basic.py",
            "--instructions=A foo that bars",
            "--model=",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("Model is empty. Cannot proceed", process.stderr)

    def test_invalid_model(self) -> None:
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_basic.py",
            "--instructions=A foo that bars",
            "--model=foobar",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn("The requested model 'foobar' does not exist.", process.stderr)

    def test_wrong_model(self) -> None:
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_basic.py",
            "--instructions=A foo that bars",
            "--model=gpt-image-1",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn(
            "The requested model 'gpt-image-1' is not supported with the Responses API.",
            process.stderr,
        )
