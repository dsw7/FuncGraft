#include "instructions.hpp"

#include "console.hpp"
#include "utils.hpp"

#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <stdexcept>

namespace {

std::string load_instructions_from_file_(const std::filesystem::path &filename)
{
    if (not std::filesystem::exists(filename)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", filename.string()));
    }

    fmt::print("Loading instructions from file '{}'\n", filename.string());
    return utils::read_from_file(filename);
}

std::string load_instructions_from_stdin_()
{
    console::print_separator();
    std::string instructions;

    while (true) {
        fmt::print(fmt::emphasis::bold, "> ");
        std::getline(std::cin, instructions);

        if (not instructions.empty()) {
            break;
        }
    }

    console::print_separator();
    return instructions;
}

} // namespace

namespace core {
namespace instructions {

std::string load_instructions(const Configurations &configs)
{
    std::string instructions;

    if (configs.instructions_from_cli) {
        instructions = configs.instructions_from_cli.value();
    } else if (configs.instructions_file) {
        instructions = load_instructions_from_file_(configs.instructions_file.value());
    } else {
        instructions = load_instructions_from_stdin_();
    }

    if (instructions.empty()) {
        throw std::runtime_error("Instructions are empty!");
    }

    return instructions;
}

} // namespace instructions
} // namespace core
