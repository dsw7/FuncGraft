#pragma once

#include <json.hpp>
#include <string>

namespace adapters {

class ErrorResponse {
public:
    ErrorResponse(const std::string &response, const int status_code);
    std::string errmsg;
    int status_code = 200;

protected:
    nlohmann::json json_;
};

struct OllamaError: public ErrorResponse {
    OllamaError(const std::string &response, const int status_code);
};

struct OpenAIError: public ErrorResponse {
    OpenAIError(const std::string &response, const int status_code);
};

} // namespace adapters
