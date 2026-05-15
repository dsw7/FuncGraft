from datetime import datetime
from pathlib import Path
from typing import Generator
from pytest import mark, fixture
from utils import assert_command_success, assert_command_failure


@mark.parametrize("option", ["-h", "--help"])
def test_help(option: str) -> None:
    assert_command_success(option)


def test_no_opts_or_args() -> None:
    assert_command_success()


def test_unknown_opt() -> None:
    stderr = assert_command_failure("-X")
    assert "Unknown option passed to command" in stderr


def test_copyright() -> None:
    stdout = assert_command_success("--help")
    assert (
        f"-- FuncGraft | Copyright (C) {datetime.now().year} by David Weber" in stdout
    )


def test_invalid_provider() -> None:
    stderr = assert_command_failure("--provider=foobar", "foobar.py")
    assert "Invalid LLM provider" in stderr


@fixture
def empty_instructions_file(text_file: Path) -> Generator[str, None, None]:
    text_file.write_text("", encoding="utf-8")
    yield str(text_file)


def test_empty_instructions_file_arg(dummy_python_file: Path) -> None:
    stderr = assert_command_failure(f"{dummy_python_file}", "--file=")
    assert "Instructions filename was not provided. Cannot proceed" in stderr


def test_missing_instructions_file(dummy_python_file: Path) -> None:
    stderr = assert_command_failure(f"{dummy_python_file}", "--file=foo.txt")
    assert "File 'foo.txt' does not exist!" in stderr


def test_empty_instructions_file(
    dummy_python_file: Path, empty_instructions_file: str
) -> None:
    stderr = assert_command_failure(
        f"{dummy_python_file}", f"--file={empty_instructions_file}"
    )
    assert "Instructions are empty!" in stderr


def test_empty_instructions(dummy_python_file: Path) -> None:
    stderr = assert_command_failure(f"{dummy_python_file}", "--instructions=")
    assert "CLI instructions are empty. Cannot proceed" in stderr


@mark.skip(reason="Test will fail because 'q' | 'quit' checked interactively")
@mark.parametrize("instruction", ["quit", "q"])
def test_exit_program(dummy_python_file: Path, instruction: str) -> None:
    stdout = assert_command_success(
        f"{dummy_python_file}", f"--instructions={instruction}"
    )
    assert "Program aborted" in stdout


@mark.parametrize(
    "provider, model, errmsg",
    [
        ("openai", "qwen3.5", "The requested model 'qwen3.5' does not exist."),
        ("ollama", "gpt-4o", "model 'gpt-4o' not found"),
    ],
)
def test_invalid_model(
    dummy_python_file: Path, provider: str, model: str, errmsg: str
) -> None:
    instruction = "Fix the ValueError"
    stderr = assert_command_failure(
        f"{dummy_python_file}",
        f"--instructions={instruction}",
        f"--provider={provider}",
        f"--model={model}",
    )
    assert errmsg in stderr


@mark.parametrize(
    "provider",
    ["openai", "ollama"],
)
def test_empty_model(dummy_python_file: Path, provider: str) -> None:
    instruction = "Fix the ValueError"
    stderr = assert_command_failure(
        f"{dummy_python_file}",
        f"--instructions={instruction}",
        f"--provider={provider}",
        "--model=",
    )
    assert "Model to override is empty" in stderr
