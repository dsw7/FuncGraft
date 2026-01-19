#include "prompt.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <json.hpp>
#include <map>
#include <optional>
#include <stdexcept>

namespace {

std::optional<std::string> resolve_label_from_extension_(const std::string &extension)
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
        return fmt::format("```\n{}```\n", body);
    }

    return fmt::format("```\n{}\n```\n", body);
}

std::string get_code_block_(const std::string &body, const std::string &label)
{
    if (body.empty()) {
        throw std::runtime_error("Body is empty. Cannot extract code block");
    }

    if (body.back() == '\n') {
        return fmt::format("```{}\n{}```\n", label, body);
    }

    return fmt::format("```{}\n{}\n```\n", label, body);
}

} // namespace

namespace prompt {

std::string build_openai_prompt(const std::string &instructions, const std::string &input_text, const std::string &extension)
{
    std::string code_block_to_edit;

    if (const auto label = resolve_label_from_extension_(extension); label.has_value()) {
        code_block_to_edit = get_code_block_(input_text, *label);
    } else {
        code_block_to_edit = get_code_block_(input_text);
    }

    const nlohmann::json example = {
        { "code", "Your updated code here" },
        { "description", "A brief explanation of the changes" }
    };

    return fmt::format(
        R"(I am editing some code. Apply the following instructions:
{}
To the following code:
{}
Return the code edits in a JSON format with keys "code" and "description." For example:
{}
)",
        get_code_block_(instructions, "plaintext"),
        code_block_to_edit,
        example.dump(4));
}

std::string build_ollama_prompt(const std::string &instructions, const std::string &input_text, const std::string &extension)
{
    std::string code_block_to_edit;

    if (const auto label = resolve_label_from_extension_(extension); label.has_value()) {
        code_block_to_edit = get_code_block_(input_text, *label);
    } else {
        code_block_to_edit = get_code_block_(input_text);
    }

    return fmt::format(R"(
I am editing some code. Apply the following instructions:
{}
To the following code:
{}
Respond using JSON.
)",
        get_code_block_(instructions, "plaintext"),
        code_block_to_edit);
}

} // namespace prompt
