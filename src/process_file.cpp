#include "process_file.hpp"

#include "utils.hpp"

#include <filesystem>
#include <fmt/core.h>
#include <stdexcept>

namespace {

std::string process_input(const std::string &input)
{
    return std::string(input);
}

} // namespace

namespace process_file {

void process_file(const cli_params::Parameters &params)
{
    if (not std::filesystem::exists(params.input_file)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", params.input_file.string()));
    }

    if (not std::filesystem::is_regular_file(params.input_file)) {
        throw std::runtime_error(fmt::format("Input '{}' is not a file!", params.input_file.string()));
    }

    const std::string input_text = utils::read_from_file(params.input_file);
    fmt::print("Loaded contents from file '{}' into memory\n", params.input_file.string());

    std::string instructions;

    if (params.instructions_file) {
        const std::filesystem::path instructions_file = params.instructions_file.value();

        if (std::filesystem::exists(instructions_file)) {
            instructions = utils::read_from_file(instructions_file);
            fmt::print("Loaded instructions from file '{}'\n", instructions_file.string());
        } else {
            throw std::runtime_error(fmt::format("File '{}' does not exist!", instructions_file.string()));
        }
    } else {
        if (params.rule) {
            instructions = params.rule.value();
            fmt::print("Loaded instructions from command line\n");
        } else {
            throw std::runtime_error("No instructions provided by file or command line");
        }
    }

    const std::string output_text = process_input(input_text);

    if (params.output_file) {
        utils::write_to_file(params.output_file.value(), output_text);
        fmt::print("Exported updated content to file '{}'\n", params.output_file.value().string());
    } else {
        fmt::print("Results:\n{}", output_text);
    }
}

} // namespace process_file
