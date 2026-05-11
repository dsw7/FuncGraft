#include "responses.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace queries {

ErrorResponse::ErrorResponse(const std::string &response, const int status_code) :
    status_code(status_code)
{
    try {
        this->json_ = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse error response: {}", e.what()));
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

SuccessResponse::SuccessResponse(const std::string &response)
{
    try {
        this->response_ = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse success response: {}", e.what()));
    }
}

OpenAIResponse::OpenAIResponse(const std::string &response) :
    SuccessResponse(response)
{
    if (not this->response_.contains("object")) {
        throw std::runtime_error("The response from OpenAI does not contain an 'object' key");
    }

    if (this->response_["object"] != "response") {
        throw std::runtime_error("The response from OpenAI is not an OpenAI Response");
    }
}

std::string OpenAIResponse::get_text_from_response_()
{
    std::string text;

    for (const auto &item: this->response_["output"]) {
        if (item["type"] == "message") {
            if (item["status"] == "completed") {
                if (item["content"][0]["type"] == "output_text") {
                    text = item["content"][0]["text"];
                    break;
                }
            }
        }
    }

    if (text.empty()) {
        throw std::runtime_error("Something went wrong. OpenAI did not return output text");
    }

    return text;
}

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

} // namespace queries
