#include "process_file.hpp"

#include "prompt.hpp"
#include "query_openai.hpp"
#include "utils.hpp"

#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <stdexcept>

namespace {

fmt::terminal_color blue = fmt::terminal_color::bright_blue;
fmt::terminal_color green = fmt::terminal_color::bright_green;

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

void print_updated_code_to_stdout(const std::string &code, const std::string &extension)
{
    const auto label = utils::resolve_label_from_extension(extension);
    std::string code_block;

    if (label) {
        code_block = utils::get_code_block(code, label.value());
    } else {
        code_block = utils::get_code_block(code);
    }

    fmt::print(fmt::emphasis::bold, "Results:\n");
    fmt::print(fg(green), "{}", code_block);
}

} // namespace

namespace process_file {

void process_file(const params::CommandLineParameters &params)
{
    utils::print_separator();
    const std::string input_text = load_input_text_from_file(params);
    const std::string instructions = load_instructions(params);
    const std::string input_file_extension = params.input_file.extension();
    const std::string prompt = prompt::build_prompt(instructions, input_text, input_file_extension);

    std::string model;

    if (params.model) {
        model = params.model.value();
    } else {
        model = "gpt-4o";
    }

    fmt::print("Using model: {}\n", model);

    if (params.verbose) {
        utils::print_separator();
        fmt::print(fmt::emphasis::bold, "Prompt:\n");
        fmt::print(fg(blue), "{}", prompt);
    }

    utils::print_separator();

    const query_openai::QueryResults results = query_openai::run_query(prompt, model);
    fmt::print(fmt::emphasis::bold, "Information:\n");
    fmt::print("Prompt tokens: {}\n", results.prompt_tokens);
    fmt::print("Completion tokens: {}\n", results.completion_tokens);
    fmt::print("Description of changes: ");
    fmt::print(fg(blue), "{}\n", results.description);

    if (params.output_file) {
        utils::write_to_file(params.output_file.value(), results.code);
        fmt::print("Exported updated content to file '{}'\n", params.output_file.value().string());
        return;
    }

    utils::print_separator();
    print_updated_code_to_stdout(results.code, input_file_extension);
    utils::print_separator();
}

} // namespace process_file
