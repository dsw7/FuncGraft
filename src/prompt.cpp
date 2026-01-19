#include "prompt.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <json.hpp>
#include <stdexcept>

namespace {

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
    std::string prompt = "I am editing some code. Apply the following instructions:\n";

    prompt += get_code_block_(instructions, "plaintext");
    prompt += "To the following code:\n";

    const auto label = utils::resolve_label_from_extension(extension);

    if (label) {
        prompt += get_code_block_(input_text, label.value());
    } else {
        prompt += get_code_block_(input_text);
    }

    prompt += "Return the code edits in a JSON format with keys \"code\" and \"description.\" For example:\n";

    const nlohmann::json example = {
        { "code", "Your updated code here" },
        { "description", "A brief explanation of the changes" }
    };

    prompt += example.dump(4) + '\n';
    return prompt;
}

std::string build_ollama_prompt(const std::string &instructions, const std::string &input_text, const std::string &extension)
{
    std::string prompt = "I am editing some code. Apply the following instructions:\n";

    prompt += get_code_block_(instructions, "plaintext");
    prompt += "To the following code:\n";

    const auto label = utils::resolve_label_from_extension(extension);

    if (label) {
        prompt += get_code_block_(input_text, label.value());
    } else {
        prompt += get_code_block_(input_text);
    }

    prompt += "Respond using JSON.\n";
    return prompt;
}

} // namespace prompt
