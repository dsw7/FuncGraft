#pragma once

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"

namespace core {
namespace reporting {
void print_query_info(const adapters::OllamaResponse &response);
void print_query_info(const adapters::OpenAIResponse &response);
} // namespace reporting
} // namespace core
