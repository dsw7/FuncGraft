# FuncGraft
Ultra lightweight CLI code editor. I tried [codex](https://github.com/openai/codex) but it felt too bloated,
led to code sprawl, and did not feel targeted enough for me. So I built my own equivalent that operates on
individual files.

🚫 _This code was not AI generated._ I do believe in leveraging AI tools for targeted code manipulations, but
I do not believe in letting AI do all the work.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Setup](#setup)

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
