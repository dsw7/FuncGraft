#pragma once

#include "responses.hpp"

#include <string>

namespace adapters {

class OpenAIResponse: public SuccessResponse {
public:
    OpenAIResponse() :
        SuccessResponse() {} // needed for variants
    OpenAIResponse(const std::string &response);

protected:
    std::string get_text_from_response_();
};

struct OpenAIClassificationResponse: public OpenAIResponse {
    OpenAIClassificationResponse() :
        OpenAIResponse() {}
    OpenAIClassificationResponse(const std::string &response);

    bool valid_instructions = false;
    std::string reasoning;
};

class OpenAIEditResponse: public OpenAIResponse {
public:
    OpenAIEditResponse() :
        OpenAIResponse() {}
    OpenAIEditResponse(const std::string &response, const double total_t);

    std::string description_of_changes;
    std::string code;

    double total_time = 0.0;
    int input_tokens = 0;
    int output_tokens = 0;

private:
    void unpack_structured_output_();
};

} // namespace adapters
