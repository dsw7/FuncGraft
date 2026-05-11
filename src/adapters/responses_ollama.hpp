#pragma once

#include "responses.hpp"

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

class OllamaEdit: public OllamaResponse {
public:
    OllamaEdit() :
        OllamaResponse() {} // needed for variants
    OllamaEdit(const std::string &response, const double total_t);

    std::string description_of_changes;
    std::string code;

    double total_time = 0.0;
    int input_tokens = 0;
    int output_tokens = 0;

private:
    void unpack_structured_output_();
};

} // namespace adapters
