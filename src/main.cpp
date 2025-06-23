#include "cli_params.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <getopt.h>
#include <stdexcept>
#include <string.h>
#include <string>

void print_help()
{
    fmt::print("Usage: gpe [OPTION]\n\n");
    fmt::print("Options:\n");
    fmt::print(" -h, --help  Print help menu\n");
}

void run_command(const std::string &command)
{
    if (command == "-h" or command == "--help") {
        print_help();
        return;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_help();
        return 0;
    }

    cli_params::Parameters params;

    while (true) {
        static struct option long_options[] = {
            { "help", no_argument, 0, 'h' },
            { "model", required_argument, 0, 'm' },
            { "output", required_argument, 0, 'o' },
            { "instructions", required_argument, 0, 'i' },
            { "rule", required_argument, 0, 'r' },
            { 0, 0, 0, 0 }
        };

        int option_index = 0;
        int opt = getopt_long(argc, argv, "hm:o:i:r:", long_options, &option_index);

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
            case 'i':
                params.instructions_file = optarg;
                break;
            case 'r':
                params.rule = optarg;
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
        run_command(argv[1]);
    } catch (const std::runtime_error &e) {
        fmt::print(stderr, fg(fmt::color::red), "{}\n", e.what());
        return 1;
    }

    return 0;
}
