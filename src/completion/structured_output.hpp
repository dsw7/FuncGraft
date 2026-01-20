#pragma once

#include <string>

namespace completion {

struct LLMResponse {
    int output_tokens = 0;
    int input_tokens = 0;
    std::string description;
    std::string output_text;
};

struct StructuredOutput {
    std::string code;
    std::string description_of_changes;
};

StructuredOutput deserialize_structured_output(const std::string &output);
} // namespace completion
