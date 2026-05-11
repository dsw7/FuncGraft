#pragma once

#include <json.hpp>
#include <string>

namespace queries {

class ErrorResponse {
public:
    ErrorResponse(const std::string &response, const int status_code);
    std::string errmsg;
    int status_code = 200;

protected:
    nlohmann::json json_;
};

struct OpenAIError: public ErrorResponse {
    OpenAIError(const std::string &response, const int status_code);
};

struct OllamaError: public ErrorResponse {
    OllamaError(const std::string &response, const int status_code);
};

class SuccessResponse {
public:
    SuccessResponse() = default;
    SuccessResponse(const std::string &response);

protected:
    nlohmann::json response_;
};

class OpenAIResponse: public SuccessResponse {
public:
    OpenAIResponse() :
        SuccessResponse() {}
    OpenAIResponse(const std::string &response);

protected:
    std::string get_text_from_response_();
};

struct OllamaResponse: public SuccessResponse {
    OllamaResponse() :
        SuccessResponse() {} // needed for variants
    OllamaResponse(const std::string &response);
};

} // namespace queries
