#pragma once

#include "responses.hpp"

#include <string>

namespace queries {

struct OllamaError: public ErrorResponse {
    OllamaError(const std::string &response, const int status_code);
};

struct OllamaResponse: public SuccessResponse {
    OllamaResponse() :
        SuccessResponse() {}
    OllamaResponse(const std::string &response);
};

} // namespace queries
