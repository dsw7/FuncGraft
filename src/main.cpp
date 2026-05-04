#include "configs.hpp"
#include "datadir.hpp"
#include "process_file.hpp"
#include "utils.hpp"

#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <getopt.h>
#include <stdexcept>
#include <string.h>
#include <string>
#include <toml.hpp>

void print_help_messages()
{
    fmt::print("-- FuncGraft | Copyright (C) {} by David Weber\n", CURRENT_YEAR);

    const std::string messages = R"(-- See https://github.com/dsw7/FuncGraft for more information

Program for editing individual files using LLMs. This program
can currently connect to LLMs served by:

  • OpenAI
  • Ollama

Usage:
  edit [OPTIONS] FILENAME

Options:
  -h, --help                       Print help menu
  -o, --output=FILE                Place output into FILE
  -f, --file=FILE                  Read instructions from FILE
  -i, --instructions=INSTRUCTIONS  Read INSTRUCTIONS via command line
  -v, --verbose                    Be more verbose with output
  -p, --provider=PROVIDER          Specify LLM provider. Valid options are [openai, ollama]

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

Configurations parse_configs_from_argv(const int argc, char **argv)
{
    Configurations configs;

    while (true) {
        static struct option long_options[] = {
            { "help", no_argument, 0, 'h' },
            { "output", required_argument, 0, 'o' },
            { "file", required_argument, 0, 'f' },
            { "instructions", required_argument, 0, 'i' },
            { "verbose", no_argument, 0, 'v' },
            { "provider", required_argument, 0, 'p' },
            { 0, 0, 0, 0 }
        };

        int option_index = 0;
        const int option = getopt_long(argc, argv, "ho:f:i:vp:", long_options, &option_index);

        if (option == -1) {
            break;
        }

        switch (option) {
            case 'h':
                print_help_messages();
                std::exit(EXIT_SUCCESS);
            case 'o':
                configs.output_file = optarg;
                break;
            case 'f':
                configs.instructions_file = optarg;
                break;
            case 'i':
                configs.instructions_from_cli = optarg;
                break;
            case 'v':
                configs.verbose = true;
                break;
            case 'p':
                configs.provider = optarg;
                break;
            default:
                fmt::print(stderr, fg(fmt::color::red), "Unknown option passed to command\n");
                std::exit(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; i++) {
        if (strcmp("edit", argv[i]) != 0) {
            configs.input_file = argv[i];
            break;
        }
    }

    return configs;
}

void load_additional_configs_from_file(Configurations &configs)
{
    const std::filesystem::path proj_config = core::datadir::get_project_data_dir() / "funcgraft.toml";

    if (not std::filesystem::exists(proj_config)) {
        throw std::runtime_error("Could not locate FuncGraft configuration file!");
    }

    toml::table table;

    try {
        table = toml::parse_file(proj_config.string());
    } catch (const toml::parse_error &e) {
        throw std::runtime_error(e);
    }

    configs.host_ollama = table["ollama"]["host"].value_or("localhost");
    configs.model_ollama = table["ollama"]["model"].value_or("gemma3:latest");
    configs.model_openai = table["openai"]["model"].value_or("gpt-4o");
    configs.port_ollama = table["ollama"]["port"].value_or(11434);

    if (not configs.provider) {
        // i.e. provider was not set via command line
        configs.provider = table["general"]["provider"].value_or("openai");
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_help_messages();
        return 0;
    }

    Configurations configs = parse_configs_from_argv(argc, argv);

    try {
        load_additional_configs_from_file(configs);
        configs.validate_configurations();
        pipeline::process_file(configs);
    } catch (const std::runtime_error &e) {
        fmt::print(stderr, fg(fmt::color::red), "{}\n", e.what());
        return 1;
    }

    return 0;
}
