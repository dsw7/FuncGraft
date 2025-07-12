#include "params.hpp"
#include "process_file.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <getopt.h>
#include <stdexcept>
#include <string.h>
#include <string>

void print_help()
{
    const std::string messages = R"(-- FuncGraft | Copyright (C) 2025 by David Weber
-- See https://github.com/dsw7/FuncGraft for more information

Program for editing individual files using LLMs.

Usage:
  edit [OPTION] 
  edit [OPTION]... FILE

Options:
  -h, --help                       Print help menu
  -m, --model=MODEL                Select model
  -o, --output=FILENAME            Place output into FILENAME
  -f, --file=FILENAME              Read instructions from FILENAME
  -i, --instructions=INSTRUCTIONS  Read INSTRUCTIONS via command line
  -v, --verbose                    Be more verbose with output

Examples:
  1. Edit a file interactively. Program will provide a [y/n] prompt asking whether to overwrite the file:
    $ edit foo.cpp
  2. Edit a file interactively and automatically overwrite file:
    $ edit foo.cpp -o foo.cpp
  3. Edit a file interactively and write to a new file:
    $ edit foo.cpp -o bar.cpp
  4. Edit a file in accordance with a long, multiline prompt:
    $ edit foo.cpp -o bar.cpp -f instructions.txt
)";

    fmt::print("{}", messages);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_help();
        return 0;
    }

    params::CommandLineParameters params;

    while (true) {
        static struct option long_options[] = {
            { "help", no_argument, 0, 'h' },
            { "model", required_argument, 0, 'm' },
            { "output", required_argument, 0, 'o' },
            { "file", required_argument, 0, 'f' },
            { "instructions", required_argument, 0, 'i' },
            { "verbose", required_argument, 0, 'v' },
            { 0, 0, 0, 0 }
        };

        int option_index = 0;
        int opt = getopt_long(argc, argv, "hm:o:f:i:v", long_options, &option_index);

        if (opt == -1) {
            break;
        }

        switch (opt) {
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
            case 'm':
                params.model = optarg;
                break;
            case 'o':
                params.output_file = optarg;
                break;
            case 'f':
                params.instructions_file = optarg;
                break;
            case 'i':
                params.instructions_from_cli = optarg;
                break;
            case 'v':
                params.verbose = true;
                break;
            default:
                print_help();
                exit(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; i++) {
        if (strcmp("edit", argv[i]) != 0) {
            params.input_file = argv[i];
            break;
        }
    }

    try {
        process_file::process_file(params);
    } catch (const std::runtime_error &e) {
        fmt::print(stderr, fg(fmt::color::red), "{}\n", e.what());
        return 1;
    }

    return 0;
}
