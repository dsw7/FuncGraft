#include "prompt.hpp"

#include <fmt/core.h>
#include <json.hpp>
#include <map>
#include <optional>

namespace {

std::optional<std::string> resolve_label_from_extension(const std::string &extension)
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

std::string get_code_block(const std::string &body, const std::optional<std::string> &label)
{
    if (label) {
        return fmt::format("```{}\n{}\n```\n", label.value(), body);
    }

    return fmt::format("```\n{}\n```\n", body);
}

} // namespace

namespace prompt {

std::string build_prompt(const std::string &instructions, const std::string &input_text, const std::string &extension)
{
    std::string prompt = "I am editing some code. Apply the following instructions:\n";

    prompt += get_code_block(instructions, "plaintext");
    prompt += "To the following code:\n";
    prompt += get_code_block(input_text, resolve_label_from_extension(extension));
    prompt += "Return the code edits in a JSON format with keys \"code\" and \"description.\" For example:\n";

    const nlohmann::json example = {
        { "code", "Your updated code here" },
        { "description", "A brief explanation of the changes" }
    };

    prompt += example.dump(4) + '\n';
    return prompt;
}

} // namespace prompt
