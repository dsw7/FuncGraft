# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# Overview

FuncGraft is an ultra-lightweight CLI code editor that uses LLMs to edit individual files. It can connect to OpenAI or Ollama providers and operates on individual files selectively.

# Commands

## Building

```bash
# Default build (format + compile and install)
make

# Compile for testing with coverage
make compile-test

# Full test suite with OpenAI
make test

# Full test suite with Ollama
make test-ollama

# Run only slow tests
make test-slow

# Clean build artifacts
make clean
```

## Formatting and Linting

```bash
# Format source files
make format

# Run clang-tidy
make tidy

# Static analysis
make lint
```

## Running Single Tests

```bash
# Run a specific test
python3 -m pytest -vs tests/test_edit.py::test_read_instructions_from_cli -k "ollama"

# Run with verbosity
python3 -m pytest -vs tests/ -k "not ollama" -m "not slow"
```

# Architecture

## High-Level Design

FuncGraft follows a layered architecture:

```
src/
├── main.cpp              # CLI entry point, argument parsing
├── configs.hpp/.cpp      # Configuration management (provider, models)
├── adapters/             # LLM provider implementations
│   ├── adapter_openai.cpp/.hpp
│   ├── adapter_ollama.cpp/.hpp
│   ├── curl_base.cpp/.hpp       # Shared curl connection logic
│   └── components.cpp/.hpp      # JSON schema generation, structured output parsing
├── pipeline/             # Core editing pipeline
│   ├── file_io.cpp/.hpp               # File read/write operations
│   ├── process_file.cpp/.hpp          # Orchestrates editing flow
│   └── text_manip.cpp/.hpp            # Code block extraction/manipulation
├── prompt/               # Prompt generation
│   ├── generate_prompt.cpp/.hpp       # Builds prompts for LLM
│   └── instructions.cpp/.hpp           # Loads instructions from CLI/files/stdin
└── utils.cpp/.hpp       # Utility functions (file I/O helpers)
```

## Data Flow

```
┌─────────────────┐    ┌──────────────────┐    ┌──────────────────┐
│   Input File    │───>│   File I/O Layer  │───>│  Process File     │
└─────────────────┘    └──────────────────┘    └──────────────────┘
                                              │
                                              v
┌─────────────────┐    ┌──────────────────┐    ┌──────────────────┐
│  Instructions   │───>│  Prompt Generator │───>│  Adapter Layer    │
└─────────────────┘    └──────────────────┘    └────────┬─────────┘
                                              │         │
                                              │         v
                                              │   ┌──────────────────┐
                                              └──>│  LLM Provider    │
                                                  │ (OpenAI/Ollama)  │
                                                  └────────┬─────────┘
                                                            │
                                                            v
┌─────────────────┐    ┌──────────────────┐    ┌──────────────────┐
│   Output File   │<───│  File I/O Layer  │<───│  Structured      │
└─────────────────┘    └──────────────────┘    │  Output Parsing   │
                                               └──────────────────┘
```

## Key Concepts

### `@ @@` Delimiters

The `@@@` delimiter wraps code regions that need editing. This enables targeted editing:

```cpp
@@@
void problematic_function()
{
    // Code that needs editing
}
@@@
```

The prompt generator wraps instructions and code blocks with Markdown-style fences, including language detection from file extensions.

### `Configurations` Class

Central configuration state passed through the pipeline. Includes:
- CLI options (verbose, output file, instructions, provider)
- Provider configuration (OpenAI model, Ollama host/port/model)
- Validation logic for required fields

### Prompt Building

`build_prompt()` in `generate_prompt.cpp`:
1. Wraps instructions in a markdown code block
2. Wraps target code with language-specific markdown fences (detected from file extension)
3. Returns a formatted prompt: "Apply the following instructions:\n{instructions}\nTo the following code:\n{code}"

# Testing

Tests are separated by provider (OpenAI vs Ollama) and include:
- `test_*.py` files
- `conftest.py` provides fixtures for temp files
- `utils.py` has helper functions for asserting command success/failure

Markers:
- `@mark.slow` - tests that take longer to run

Test utilities:
- `assert_command_success()` - asserts return code 0
- `assert_command_failure()` - asserts return code 1
- `assert_python_script_runs()` - runs edited python files to verify edits

# Common Issues

### Invalid Provider
Check `funcgraft.toml` or `--provider` flag for valid values: `openai` or `ollama`.

### Missing Configuration File
The program looks for `funcgraft.toml` in `~/.funcgraft/`. After installation:
```bash
cp -rv .funcgraft/ ~
```

### Empty Delimited Block
The `@@@` delimiters must surround actual code. Empty blocks or adjacent delimiters without code will fail.

### File Not Found Errors
The program validates that input and output files exist before proceeding.

# Dependencies

- CMake
- {fmt} library
- libcurl
- nlohmann/json.hpp (bundled in external/)
- toml++ (bundled in external/)

# Vim Integration

For wrapping code with `@ @@` markers in Vim, add to `.vimrc`:

```vim
function! WrapCodeWithFuncGraftDelimiters()
  let l:start_line = getpos("'<")[1]
  let l:end_line = getpos("'>")[1]
  execute l:end_line . 'put ="\n"'
  execute l:start_line . ',' . l:end_line . 'move ' . (l:end_line + 1)
  execute l:start_line . 'put ="\n@@@"\n"'
  execute (l:end_line + 2) . 'put ="\n@@@\n"'
  execute l:start_line . 'd'
endfunction
```

```vim
xnoremap ed :<C-u>call WrapCodeWithFuncGraftDelimiters()<CR>
```
