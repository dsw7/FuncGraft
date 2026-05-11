#include "responses_openai.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace adapters {

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

} // namespace adapters
