#include "prompt.hpp"

#include <fmt/core.h>
#include <json.hpp>
#include <optional>

namespace {

std::string get_code_block(const std::string &body, const std::optional<std::string> &label = std::nullopt)
{
    if (label) {
        return fmt::format("```{}\n{}\n```\n", label.value(), body);
    }

    return fmt::format("```\n{}\n```\n", body);
}

} // namespace

namespace prompt {

std::string build_prompt(const params::InternalParameters &params)
{
    std::string prompt = "I am editing some code. Apply the following instructions:\n";

    prompt += get_code_block(params.instructions, "plaintext");
    prompt += "To the following code:\n";
    prompt += get_code_block(params.input_text);
    prompt += "Return the code edits in a JSON format with keys \"code\" and \"description.\" For example:\n";

    const nlohmann::json example = {
        { "code", "Your updated code here" },
        { "description", "A brief explanation of the changes" }
    };

    prompt += example.dump(4) + '\n';
    return prompt;
}

} // namespace prompt
