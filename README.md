# FuncGraft
Ultra lightweight CLI code editor. I tried [codex](https://github.com/openai/codex) but it felt too bloated,
led to code sprawl, and did not feel targeted enough for me. So I built my own equivalent that operates on
individual files.

🚫 _This code was not AI generated._ I do believe in leveraging AI tools for targeted code manipulations, but
I do not believe in letting AI do all the work.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Setup](#setup)
- [Usage](#usage)

## Prerequisites
Ensure you possess a valid OpenAI API key. Set it as an environment variable:
```bash
export OPENAI_API_KEY="<your-api-key>"
```
This program requires [CMake](https://cmake.org/), [{fmt}](https://fmt.dev/latest/) and
[libcurl](https://curl.se/libcurl/). These can be installed as follows:

### Ubuntu/Debian
```console
apt install cmake libfmt-dev libcurl4-openssl-dev
```
### macOS
```console
brew install cmake fmt
# libcurl usually comes bundled with macOS
```
### Other systems
This program should work on other Unix-like systems (i.e. other Linux distributions) however I do not
extensively test these.

## Setup
Compile the binary by executing the `make` target:
```console
make compile
```
The binary will be installed into the directory specified by CMake's [install()
function](https://cmake.org/cmake/help/latest/command/install.html#command:install). To clean up generated
artifacts:
```console
make clean
```

## Usage

### Example
Assume `foo.cpp` uses CamelCase formatting and needs to be converted to snake_case. Outline these changes in a
simple text file, such as `edit.txt`:
```plaintext
Convert all code from CamelCase to snake_case.
```
Then run:
```console
edit foo.cpp -f edit.txt
```
This will print the updated code to `stdout` and prompt whether to overwrite `foo.cpp`. To automatically
overwrite `foo.cpp`, simply run:
```console
edit foo.cpp -i edit.txt -o foo.cpp
```
To save the updated code to a new file, such as `bar.cpp`, execute:
```console
edit foo.cpp -i edit.txt -o bar.cpp
```
> [!NOTE]
> The instructions in `edit.txt` do not require prompt engineering. When processed, these instructions
> are combined with additional context and output format specifications to create a complete prompt.

### Debugging
Run the program with the `-v` flag to enable verbosity:
```console
edit foo.cpp -f edit.txt -v
```
This will print out the prompt being sent to OpenAI.

### Specify a custom model
Run the program with the `-m` or `--model` option:
```console
edit foo.cpp -f edit.txt -m <your-model>
```
