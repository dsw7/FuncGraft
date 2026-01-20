#include "configs.hpp"
#include "params.hpp"
#include "process_file.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <getopt.h>
#include <stdexcept>
#include <string.h>
#include <string>

void print_help_messages()
{
    fmt::print("-- FuncGraft | Copyright (C) {} by David Weber\n", CURRENT_YEAR);

    const std::string messages = R"(-- See https://github.com/dsw7/FuncGraft for more information

Program for editing individual files using LLMs. This program
can currently connect to LLMs served by:
  - OpenAI
  - Ollama

Usage:
  edit [OPTIONS] FILENAME

Options:
  -h, --help                       Print help menu
  -o, --output=FILE                Place output into FILE
  -f, --file=FILE                  Read instructions from FILE
  -i, --instructions=INSTRUCTIONS  Read INSTRUCTIONS via command line
  -l, --use-local                  Connect to local LLM
  -v, --verbose                    Be more verbose with output

Examples:
  > Edit a file interactively. Program will provide a [y/n] prompt asking whether to overwrite the file:
    $ edit foo.cpp
  > Edit a file interactively and automatically overwrite file:
    $ edit foo.cpp -o foo.cpp
  > Edit a file interactively and write to a new file:
    $ edit foo.cpp -o bar.cpp
  > Edit a file in accordance with a long, multiline prompt:
    $ edit foo.cpp -o bar.cpp -f instructions.txt

)";

    fmt::print("{}", messages);
}

params::CommandLineParameters parse_opts_from_argv(const int argc, char **argv)
{
    params::CommandLineParameters params;

    while (true) {
        static struct option long_options[] = {
            { "help", no_argument, 0, 'h' },
            { "output", required_argument, 0, 'o' },
            { "file", required_argument, 0, 'f' },
            { "instructions", required_argument, 0, 'i' },
            { "use-local", no_argument, 0, 'l' },
            { "verbose", no_argument, 0, 'v' },
            { 0, 0, 0, 0 }
        };

        int option_index = 0;
        const int option = getopt_long(argc, argv, "ho:f:i:lv", long_options, &option_index);

        if (option == -1) {
            break;
        }

        switch (option) {
            case 'h':
                print_help_messages();
                std::exit(EXIT_SUCCESS);
            case 'o':
                params.output_file = optarg;
                break;
            case 'f':
                params.instructions_file = optarg;
                break;
            case 'i':
                params.instructions_from_cli = optarg;
                break;
            case 'l':
                params.use_local_llm = true;
                break;
            case 'v':
                params.verbose = true;
                break;
            default:
                fmt::print(stderr, fg(fmt::color::red), "Unknown option passed to command\n");
                std::exit(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; i++) {
        if (strcmp("edit", argv[i]) != 0) {
            params.input_file = argv[i];
            break;
        }
    }

    return params;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_help_messages();
        return 0;
    }

    params::CommandLineParameters params = parse_opts_from_argv(argc, argv);

    try {
        params.validate_params();
    } catch (const std::invalid_argument &e) {
        fmt::print(stderr, fg(fmt::color::red), "{}\n", e.what());
        return 1;
    }

    try {
        configs.load_configs_from_config_file();
        pipeline::process_file(params);
    } catch (const std::runtime_error &e) {
        fmt::print(stderr, fg(fmt::color::red), "{}\n", e.what());
        return 1;
    }

    return 0;
}
