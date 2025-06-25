#include "instructions.hpp"

#include "utils.hpp"

#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <stdexcept>

namespace {

std::string load_instructions_from_file(const std::filesystem::path &filename)
{
    if (not std::filesystem::exists(filename)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", filename.string()));
    }

    fmt::print("Loading instructions from file '{}'\n", filename.string());
    return utils::read_from_file(filename);
}

std::string load_instructions_from_stdin()
{
    utils::print_separator();
    fmt::print(fmt::emphasis::bold, "Input: ");

    std::string instructions;
    std::getline(std::cin, instructions);
    return instructions;
}

} // namespace

namespace instructions {

std::string load_instructions(const params::CommandLineParameters &params)
{
    std::string instructions;

    if (params.instructions_from_cli) {
        instructions = params.instructions_from_cli.value();
    } else if (params.instructions_file) {
        instructions = load_instructions_from_file(params.instructions_file.value());
    } else {
        instructions = load_instructions_from_stdin();
    }

    if (instructions.empty()) {
        throw std::runtime_error("Instructions are empty!");
    }

    return instructions;
}

} // namespace instructions
