#pragma once

#include "responses.hpp"

#include <json.hpp>
#include <string>

namespace adapters {

struct OllamaResponse: public SuccessResponse {
    OllamaResponse() :
        SuccessResponse() {}
    OllamaResponse(const std::string &response);
};

} // namespace adapters
