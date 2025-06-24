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
    fmt::print("Usage: edit [OPTION] file\n\n");
    fmt::print("Options:\n");
    fmt::print(" -h, --help                        Print help menu\n");
    fmt::print(" -m <model>, --model=<model>       Select model\n");
    fmt::print(" -o <file>, --output=<file>        Place output into <file>\n");
    fmt::print(" -f <file>, --file=<file>          Read instructions from <file>\n");
    fmt::print(" -i <text>, --instructions=<text>  Read instructions via command line\n");
    fmt::print(" -v, --verbose                     Be more verbose with output\n");
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
