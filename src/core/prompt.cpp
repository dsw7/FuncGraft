#include "prompt.hpp"

#include "utils.hpp"

#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <json.hpp>
#include <map>
#include <optional>
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
    utils::print_separator();
    std::string instructions;

    while (true) {
        fmt::print(fmt::emphasis::bold, "> ");
        std::getline(std::cin, instructions);

        if (not instructions.empty()) {
            break;
        }
    }

    utils::print_separator();
    return instructions;
}

std::string load_instructions_(const Configurations &configs)
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

std::optional<std::string> resolve_label_from_extension_(const std::filesystem::path &filename)
{
    static std::map<std::string, std::string> ext_to_label {
        { ".bash", "bash" },
        { ".sh", "sh" },
        { ".c", "c" },
        { ".cpp", "cpp" },
        { ".c++", "cpp" },
        { ".cs", "csharp" },
        { ".csharp", "csharp" },
        { ".css", "css" },
        { ".html", "html" },
        { ".javascript", "javascript" },
        { ".js", "javascript" },
        { ".json", "json" },
        { ".java", "java" },
        { ".kotlin", "kotlin" },
        { ".perl", "perl" },
        { ".php", "php" },
        { ".python", "python" },
        { ".py", "python" },
        { ".ruby", "ruby" },
        { ".rust", "rust" },
        { ".sql", "sql" },
        { ".swift", "swift" },
        { ".typescript", "typescript" },
        { ".ts", "typescript" },
        { ".xml", "xml" },
        { ".yaml", "yaml" },
        { ".yml", "yaml" },
    };

    const std::string extension = filename.extension();

    if (ext_to_label.contains(extension)) {
        return ext_to_label[extension];
    }

    return std::nullopt;
}

std::string get_code_block_(const std::string &body)
{
    if (body.empty()) {
        throw std::runtime_error("Body is empty. Cannot extract code block");
    }

    if (body.back() == '\n') {
        return fmt::format("```\n{}```", body);
    }

    return fmt::format("```\n{}\n```", body);
}

std::string get_code_block_(const std::string &body, const std::string &label)
{
    if (body.empty()) {
        throw std::runtime_error("Body is empty. Cannot extract code block");
    }

    if (body.back() == '\n') {
        return fmt::format("```{}\n{}```", label, body);
    }

    return fmt::format("```{}\n{}\n```", label, body);
}

} // namespace

namespace core {
namespace prompt {

std::string build_prompt(const Configurations &configs, const std::string &input_text)
{
    const std::string instructions = load_instructions_(configs);
    std::string code_block_to_edit;

    if (const auto label = resolve_label_from_extension_(configs.input_file); label.has_value()) {
        code_block_to_edit = get_code_block_(input_text, *label);
    } else {
        code_block_to_edit = get_code_block_(input_text);
    }

    return fmt::format(R"(Apply the following instructions:
{}
To the following code:
{}
)",
        get_code_block_(instructions, "plaintext"),
        code_block_to_edit);
}

} // namespace prompt
} // namespace core
