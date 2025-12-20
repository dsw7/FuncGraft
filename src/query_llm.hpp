#pragma once

#include <string>

namespace query_llm {

struct LLMResponse {
    int output_tokens = 0;
    int input_tokens = 0;
    std::string description;
    std::string output_text;
};

LLMResponse run_openai_query(const std::string &prompt);
LLMResponse run_ollama_query(const std::string &prompt);

} // namespace query_llm
