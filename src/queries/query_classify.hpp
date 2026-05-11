#pragma once

#include "adapter_ollama.hpp"
#include "configs.hpp"
#include "responses_ollama.hpp"

#include <expected>
#include <string>

namespace queries {

struct OllamaClassifier: public adapters::Ollama {
    OllamaClassifier(const Configurations &configs) :
        adapters::Ollama(configs) {}

    std::expected<adapters::OllamaClassification, adapters::OllamaError> classify_instructions(const std::string &prompt);
};

} // namespace queries
