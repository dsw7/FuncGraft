#pragma once

#include "configs.hpp"
#include "curl_base.hpp"

#include <expected>
#include <json.hpp>
#include <string>

namespace adapters {

class OpenAIResponse {
public:
    OpenAIResponse(const std::string &response);

    bool was_refused = false;
    std::string description;
    std::string output_text;

    double total_time = 0.0;
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
    OpenAI(const Configurations &configs);
    std::expected<OpenAIResponse, OpenAIError> query_messages_api(const std::string &prompt);

private:
    std::string model_;
};

} // namespace adapters
