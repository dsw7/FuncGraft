#pragma once

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "configs.hpp"
#include "responses.hpp"

#include <expected>
#include <string>

namespace queries {

struct OpenAIClassification: public OpenAIResponse {
    OpenAIClassification() :
        OpenAIResponse() {}
    OpenAIClassification(const std::string &response);

    bool valid_instructions = false;
    std::string reasoning;
};

struct OllamaClassification: public OllamaResponse {
    OllamaClassification(const std::string &response);

    bool valid_instructions = false;
    std::string reasoning;
};

struct OpenAIClassifier: public adapters::OpenAI {
    OpenAIClassifier(const Configurations &configs) :
        adapters::OpenAI(configs) {}

    std::expected<OpenAIClassification, OpenAIError> classify_instructions(const std::string &prompt);
};

struct OllamaClassifier: public adapters::Ollama {
    OllamaClassifier(const Configurations &configs) :
        adapters::Ollama(configs) {}

    std::expected<OllamaClassification, OllamaError> classify_instructions(const std::string &prompt);
};

} // namespace queries
