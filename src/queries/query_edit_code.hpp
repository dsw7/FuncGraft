#pragma once

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "configs.hpp"
#include "responses.hpp"

#include <expected>
#include <string>

namespace queries {

class OpenAIEdit: public OpenAIResponse {
public:
    OpenAIEdit() :
        OpenAIResponse() {}
    OpenAIEdit(const std::string &response, const double total_t);

    std::string description_of_changes;
    std::string code;

    double total_time = 0.0;
    int input_tokens = 0;
    int output_tokens = 0;

private:
    void unpack_structured_output_();
};

class OllamaEdit: public OllamaResponse {
public:
    OllamaEdit() :
        OllamaResponse() {} // needed for variants
    OllamaEdit(const std::string &response, const double total_t);

    std::string description_of_changes;
    std::string code;

    double total_time = 0.0;
    int input_tokens = 0;
    int output_tokens = 0;

private:
    void unpack_structured_output_();
};

struct OpenAICodeEditor: public adapters::OpenAI {
    OpenAICodeEditor(const Configurations &configs) :
        adapters::OpenAI(configs) {}

    std::expected<OpenAIEdit, OpenAIError> edit_code(const std::string &prompt);
};

struct OllamaCodeEditor: public adapters::Ollama {
    OllamaCodeEditor(const Configurations &configs) :
        adapters::Ollama(configs) {}

    std::expected<OllamaEdit, OllamaError> edit_code(const std::string &prompt);
};

} // namespace queries
