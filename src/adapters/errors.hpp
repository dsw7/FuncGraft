#pragma once

#include <string>

namespace adapters {

struct OllamaError {
    OllamaError(const std::string &response, const int status_code);

    std::string errmsg;
    int status_code = 200;
};

struct OpenAIError {
    OpenAIError(const std::string &response, const int status_code);

    std::string errmsg;
    int status_code = 200;
};

} // namespace adapters
