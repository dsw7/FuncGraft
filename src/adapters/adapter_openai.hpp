#pragma once

#include "configs.hpp"
#include "curl_base.hpp"

#include <expected>
#include <json.hpp>
#include <string>

namespace adapters {

class OpenAIResponse {
public:
    OpenAIResponse() = default;
    OpenAIResponse(const std::string &response);

protected:
    std::string get_text_from_response_();
    nlohmann::json response_;
};

struct OpenAIClassificationResponse: public OpenAIResponse {
    OpenAIClassificationResponse() :
        OpenAIResponse() {} // needed for variants
    OpenAIClassificationResponse(const std::string &response);

    bool valid_instructions = false;
    std::string reasoning;
};

class OpenAIEditResponse: public OpenAIResponse {
public:
    OpenAIEditResponse() :
        OpenAIResponse() {} // needed for variants
    OpenAIEditResponse(const std::string &response, const double total_t);

    std::string description;
    std::string output_text;

    double total_time = 0.0;
    int input_tokens = 0;
    int output_tokens = 0;

private:
    void unpack_structured_output_();
};

struct OpenAIError {
    OpenAIError(const std::string &response, const int status_code);

    std::string errmsg;
    int status_code = 200;
};

class OpenAI: public CurlBase {
public:
    OpenAI(const Configurations &configs);
    std::expected<OpenAIClassificationResponse, OpenAIError> classify_instructions(const std::string &prompt);
    std::expected<OpenAIEditResponse, OpenAIError> query_edit_code(const std::string &prompt);

private:
    std::string query_responses_api_(const std::string &post_fields);

    std::string model_;
};

} // namespace adapters
