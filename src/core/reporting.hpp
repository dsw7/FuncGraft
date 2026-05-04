#pragma once

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"

#include <string>

namespace core {
namespace reporting {
void print_prompt(const std::string &prompt);
void print_code_being_targeted(const std::string &code);
void print_query_info(const adapters::OllamaResponse &response);
void print_query_info(const adapters::OpenAIResponse &response);
} // namespace reporting
} // namespace core
