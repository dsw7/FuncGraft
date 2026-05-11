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
├── configs.cpp/.hpp      # Configuration management (provider, models)
├── adapters/             # LLM provider implementations (curl-based HTTP client)
│   ├── adapter_openai.cpp/.hpp
│   ├── adapter_ollama.cpp/.hpp
│   ├── curl_base.cpp/.hpp
├── core/                 # Core domain logic
│   ├── code.cpp/.hpp     # Code block extraction/manipulation
│   ├── datadir.cpp/.hpp  # Data directory management
│   └── run_queries.cpp/.hpp  # Threading support for API queries
├── queries/              # Query implementations (classification and code editing)
│   ├── query_classify.cpp/.hpp   # Instruction classification queries
│   ├── query_edit_code.cpp/.hpp  # Code editing queries
│   └── responses.cpp/.hpp        # Response parsing (OpenAI/Ollama)
├── process_file.cpp/.hpp # Orchestrates editing flow
├── utils.cpp/.hpp        # Utility functions (file I/O helpers)
└── external/             # Third-party dependencies
    ├── json.hpp          # nlohmann/json
    └── toml.hpp          # toml++
```

## Key Concepts

### `@@@` Delimiters

The `@@@` delimiters wrap code regions that need editing. This enables targeted editing:

```cpp
@@@
void problematic_function()
{
    // Code that needs editing
}
@@@
```

The code module detects these delimiters and extracts the code block for processing. In test mode, the edited output includes diff markers (`<<<<<<<`, `=======`, `>>>>>>>`).

### `Configurations` Class

Central configuration state passed through the pipeline. Includes:
- CLI options (verbose, output file, instructions, provider)
- Provider configuration (OpenAI model, Ollama host/port/model)
- Validation logic for required fields

### Threading

`run_queries.cpp/.hpp` (core/run_queries.{cpp,hpp}):
- Manages timer threads with visual spinners during long API waits
- Uses `std::atomic<bool>` for atomic flag operations
- Runs timer thread in parallel with the actual API query, prints spinner in foreground
- Handles errors and rethrows after query completes
- Exports functions: `run_openai_query()` and `run_ollama_query()`

### Prompt Building

Prompts are generated inline within the query implementations:

1. **Classification prompts** (`query_classify.cpp`):
   - System prompt: Classifies whether user input is a programming-related request
   - User prompt: Wraps instructions in `<input>...</input>` tags
   - Structured output schema for `reasoning` (string) and `valid_instructions` (boolean)

2. **Code editing prompts** (`query_edit_code.cpp`):
   - System prompt: Assistant persona specialized in programming edits
   - User prompt: Wraps instructions as plaintext and code with language-specific markdown fences
   - Structured output schema for `description_of_changes` (string) and `code` (string)

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

For wrapping code with `@@@` markers in Vim, add to `.vimrc`:

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
