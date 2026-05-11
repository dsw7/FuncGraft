#pragma once

#include "configs.hpp"
#include "curl_base.hpp"
#include "errors.hpp"
#include "responses_ollama.hpp"

#include <expected>
#include <json.hpp>
#include <string>

namespace adapters {

class Ollama: public CurlBase {
public:
    Ollama(const Configurations &configs);
    std::expected<OllamaClassification, OllamaError> classify_instructions(const std::string &prompt);
    std::expected<OllamaEditResponse, OllamaError> query_edit_code(const std::string &prompt);

private:
    std::string query_chat_api_(const std::string &post_fields);

    std::string model_;
    std::string host_ollama_;
    int port_ollama_ = 11434;
};

} // namespace adapters
