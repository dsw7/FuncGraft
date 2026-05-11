#pragma once

#include "responses.hpp"

#include <string>

namespace queries {

struct OpenAIError: public ErrorResponse {
    OpenAIError(const std::string &response, const int status_code);
};

class OpenAIResponse: public SuccessResponse {
public:
    OpenAIResponse() :
        SuccessResponse() {} // needed for variants
    OpenAIResponse(const std::string &response);

protected:
    std::string get_text_from_response_();
};

} // namespace queries
