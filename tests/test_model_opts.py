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
        self.assertIn(
            "The model `foobar` does not exist or you do not have access to it.",
            process.stderr,
        )

    def test_wrong_model(self) -> None:
        command = [
            get_gpe_binary(),
            LOC_TEST_DATA / "dummy_basic.py",
            "--instructions=A foo that bars",
            "--model=tts-1-hd",
        ]
        process = run(command, stdout=PIPE, stderr=PIPE, text=True)
        self.assertEqual(process.returncode, 1)
        self.assertIn(
            "This is not a chat model and thus not supported in the v1/chat/completions endpoint. Did you mean to use v1/completions?",
            process.stderr,
        )
