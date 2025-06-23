#include "process_file.hpp"

#include "utils.hpp"

#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <stdexcept>

namespace {

fmt::terminal_color blue = fmt::terminal_color::bright_blue;

std::string load_input_text_from_file(const params::CommandLineParameters &params)
{
    if (not std::filesystem::exists(params.input_file)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", params.input_file.string()));
    }

    if (not std::filesystem::is_regular_file(params.input_file)) {
        throw std::runtime_error(fmt::format("Input '{}' is not a file!", params.input_file.string()));
    }

    fmt::print("Loading contents from file '{}' into memory\n", params.input_file.string());
    return utils::read_from_file(params.input_file);
}

std::string load_instructions(const params::CommandLineParameters &params)
{
    if (params.rule) {
        fmt::print("Loading instructions from command line\n");
        return params.rule.value();
    }

    if (not params.instructions_file) {
        throw std::runtime_error("No instructions provided by file or command line");
    }

    const std::string instructions_file = params.instructions_file.value().string();

    if (not std::filesystem::exists(instructions_file)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", instructions_file));
    }

    fmt::print("Loading instructions from file '{}'\n", instructions_file);
    return utils::read_from_file(instructions_file);
}

std::string build_prompt(const params::InternalParameters &internal_params)
{
    return fmt::format(
        "I am editing some code. Apply the following instructions:\n"
        "```plaintext\n{}\n```\n"
        "To the following code:\n```\n{}\n```\n"
        "Please return the code edits in a JSON format with keys \"code\" and \"description.\" "
        "For example:\n"
        "{{\n"
        "  \"code\": \"Your updated code here\",\n"
        "  \"description\": \"A brief explanation of the changes\",\n"
        "}}\n",
        internal_params.instructions, internal_params.input_text);
}

} // namespace

namespace process_file {

void process_file(const params::CommandLineParameters &cli_params)
{
    params::InternalParameters internal_params;

    internal_params.input_text = load_input_text_from_file(cli_params);
    internal_params.instructions = load_instructions(cli_params);

    const std::string prompt = build_prompt(internal_params);
    fmt::print("The prompt was:\n");
    fmt::print(fg(blue), "{}", prompt);

    const std::string output_text = internal_params.input_text;

    if (cli_params.output_file) {
        utils::write_to_file(cli_params.output_file.value(), output_text);
        fmt::print("Exported updated content to file '{}'\n", cli_params.output_file.value().string());
    } else {
        fmt::print("Results:\n{}", output_text);
    }
}

} // namespace process_file
