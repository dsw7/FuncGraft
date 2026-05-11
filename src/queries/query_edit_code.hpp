#pragma once

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "configs.hpp"
#include "responses_ollama.hpp"
#include "responses_openai.hpp"

#include <expected>
#include <string>

namespace queries {

struct OpenAICodeEditor: public adapters::OpenAI {
    OpenAICodeEditor(const Configurations &configs) :
        adapters::OpenAI(configs) {}

    std::expected<adapters::OpenAIEdit, adapters::OpenAIError> edit_code(const std::string &prompt);
};

struct OllamaCodeEditor: public adapters::Ollama {
    OllamaCodeEditor(const Configurations &configs) :
        adapters::Ollama(configs) {}

    std::expected<adapters::OllamaEdit, adapters::OllamaError> edit_code(const std::string &prompt);
};

} // namespace queries
