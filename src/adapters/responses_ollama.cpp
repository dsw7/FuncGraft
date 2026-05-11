#include "responses_ollama.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace adapters {

OllamaResponse::OllamaResponse(const std::string &response) :
    SuccessResponse(response)
{
    if (not this->response_.contains("done")) {
        throw std::runtime_error("The response from Ollama does not contain the 'done' key");
    }

    if (not this->response_["done"]) {
        throw std::runtime_error("The response from Ollama indicates the job is not done");
    }
}

OllamaClassification::OllamaClassification(const std::string &response) :
    OllamaResponse(response)
{
    nlohmann::json structured_output;

    try {
        const std::string content = this->response_["message"]["content"];
        structured_output = nlohmann::json::parse(content);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse structured output: {}", e.what()));
    }

    this->valid_instructions = structured_output.at("valid_instructions").get<bool>();
    this->reasoning = structured_output["reasoning"];
}

} // namespace adapters
