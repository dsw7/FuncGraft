#pragma once

#include "responses.hpp"

#include <json.hpp>
#include <string>

namespace adapters {

class OpenAIResponse: public SuccessResponse {
public:
    OpenAIResponse() :
        SuccessResponse() {}
    OpenAIResponse(const std::string &response);

protected:
    std::string get_text_from_response_();
};

} // namespace adapters
