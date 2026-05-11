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

struct OllamaClassification: public OllamaResponse {
    OllamaClassification(const std::string &response);

    bool valid_instructions = false;
    std::string reasoning;
};

} // namespace adapters
