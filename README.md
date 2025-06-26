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

### Example - read from stdin
Assume `foo.cpp` uses CamelCase formatting and needs to be converted to snake_case. Run:
```console
edit foo.cpp
```
Which will begin a basic interactive session. Outline the required changes when prompted:
```plaintext
Input: Convert all code from CamelCase to snake_case.
```
This will print the updated code to `stdout` and prompt whether to overwrite `foo.cpp`. To automatically
overwrite `foo.cpp`, simply run:
```console
edit foo.cpp -o foo.cpp
```
To save the updated code to a new file, such as `bar.cpp`, execute:
```console
edit foo.cpp -o bar.cpp
```

### Example - working with complex prompts
Complex multiline prompts can be written into a file and read into the program. For example:
```bash
echo "Convert all code from CamelCase to snake_case." > edit.txt && \
edit /tmp/test.py -o /tmp/test.py -f edit.txt
```
> [!NOTE]
> The instructions in `edit.txt` do not require prompt engineering. When processed, these instructions
> are combined with additional context and output format specifications to create a complete prompt.

### Example - targeted code manipulation
Suppose we have the file `example.c`:
```c
#include <stdio.h>

void printIntAddr()
{
    const int i = 42;
    printf("Value: %d\n", i);
    printf("Address: %p\n", (void *)&i);
}

void print_float_addr()
{
    const float f = 3.14f;
    printf("Value: %.2f\n", f);
    printf("Address: %p\n", (void *)&f);
}

int main()
{
    print_int_addr();
    print_float_addr();
    return 0;
}
```
This code will not compile. The case between the two functions that print an integer address are mixed. To
both resolve this and minimize token usage, the offending code can be isolated with `@@@@@` delimiters:
```c
#include <stdio.h>

@@@@@
void printIntAddr()
{
    const int i = 42;
    printf("Value: %d\n", i);
    printf("Address: %p\n", (void *)&i);
}
@@@@@

void print_float_addr()
{
...
```
We can then run:
```console
edit example.c
```
And request a CamelCase to snake_case conversion, thus resolving the missing function definition.

### Debugging
Run the program with the `-v` flag to enable verbosity:
```console
edit foo.cpp -v
```
This will print out the prompt being sent to OpenAI.

### Specify a custom model
Run the program with the `-m` or `--model` option:
```console
edit foo.cpp -m <your-model>
```
