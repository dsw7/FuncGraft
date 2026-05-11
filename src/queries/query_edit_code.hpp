#pragma once

#include "adapter_ollama.hpp"
#include "configs.hpp"
#include "responses_ollama.hpp"

#include <expected>
#include <string>

namespace queries {

struct OllamaCodeEditor: public adapters::Ollama {
    OllamaCodeEditor(const Configurations &configs) :
        adapters::Ollama(configs) {}

    std::expected<adapters::OllamaEdit, adapters::OllamaError> edit_code(const std::string &prompt);
};

} // namespace queries
