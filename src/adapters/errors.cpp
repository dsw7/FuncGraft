#include "errors.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace adapters {

ErrorResponse::ErrorResponse(const std::string &response, const int status_code) :
    status_code(status_code)
{
    try {
        this->json_ = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse response: {}", e.what()));
    }
}

OpenAIError::OpenAIError(const std::string &response, const int status_code) :
    ErrorResponse(response, status_code)
{
    if (not this->json_.contains("error")) {
        throw std::runtime_error("An error occurred but 'error' key not found in the response JSON");
    }

    if (not this->json_["error"].contains("message")) {
        throw std::runtime_error("An error occurred but 'error.message' not found in the response JSON");
    }

    this->errmsg = this->json_["error"]["message"];
}

OllamaError::OllamaError(const std::string &response, const int status_code) :
    ErrorResponse(response, status_code)
{
    if (not this->json_.contains("error")) {
        throw std::runtime_error("An error occurred but 'error' key not found in the response JSON");
    }

    this->errmsg = this->json_["error"];
}

} // namespace adapters
