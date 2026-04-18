#pragma once

#include "curl_base.hpp"

#include <expected>
#include <string>

namespace adapters {

struct OllamaResponse {
    OllamaResponse(const std::string &response);

    std::string description;
    std::string output_text;

    int input_tokens = 0;
    int output_tokens = 0;
};

struct OllamaError {
    OllamaError(const std::string &response, const int status_code);

    std::string errmsg;
    int status_code = 200;
};

class Ollama: public CurlBase {
public:
    std::expected<OllamaResponse, OllamaError> query_generate_api(const std::string &prompt);
};

} // namespace adapters
