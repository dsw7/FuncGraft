#include "prompt.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <json.hpp>

namespace prompt {

std::string build_openai_prompt(const std::string &instructions, const std::string &input_text, const std::string &extension)
{
    std::string prompt = "I am editing some code. Apply the following instructions:\n";

    prompt += utils::get_code_block(instructions, "plaintext");
    prompt += "To the following code:\n";

    const auto label = utils::resolve_label_from_extension(extension);

    if (label) {
        prompt += utils::get_code_block(input_text, label.value());
    } else {
        prompt += utils::get_code_block(input_text);
    }

    prompt += "Return the code edits in a JSON format with keys \"code\" and \"description.\" For example:\n";

    const nlohmann::json example = {
        { "code", "Your updated code here" },
        { "description", "A brief explanation of the changes" }
    };

    prompt += example.dump(4) + '\n';
    return prompt;
}

} // namespace prompt
