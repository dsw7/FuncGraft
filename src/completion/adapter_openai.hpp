#pragma once

#include "curl_base.hpp"

#include <expected>
#include <json.hpp>
#include <string>

namespace completion {

class OpenAIResponse {
public:
    OpenAIResponse(const std::string &response);

    std::string description;
    std::string output_text;

    int input_tokens = 0;
    int output_tokens = 0;

private:
    std::string extract_output_from_response_();
    nlohmann::json response_;
};

struct OpenAIError {
    OpenAIError(const std::string &response, const int status_code);

    std::string errmsg;
    int status_code = 200;
};

class OpenAI: public CurlBase {
public:
    std::expected<OpenAIResponse, OpenAIError> query_messages_api(const std::string &prompt);
};

} // namespace completion
