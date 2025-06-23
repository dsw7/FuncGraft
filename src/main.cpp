#include <fmt/color.h>
#include <fmt/core.h>
#include <stdexcept>
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

    try {
        run_command(argv[1]);
    } catch (const std::runtime_error &e) {
        fmt::print(stderr, fg(fmt::color::red), "{}\n", e.what());
        return 1;
    }

    return 0;
}
