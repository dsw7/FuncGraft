#pragma once

#include "configs.hpp"
#include "curl_base.hpp"
#include "errors.hpp"
#include "responses_openai.hpp"

#include <expected>
#include <json.hpp>
#include <string>

namespace adapters {

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

    std::string description_of_changes;
    std::string code;

    double total_time = 0.0;
    int input_tokens = 0;
    int output_tokens = 0;

private:
    void unpack_structured_output_();
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
