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

OpenAIClassification::OpenAIClassification(const std::string &response) :
    OpenAIResponse(response)
{
    nlohmann::json structured_output;

    try {
        const std::string text = this->get_text_from_response_();
        structured_output = nlohmann::json::parse(text);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse structured output: {}", e.what()));
    }

    this->valid_instructions = structured_output.at("valid_instructions").get<bool>();
    this->reasoning = structured_output["reasoning"];
}

OpenAIEdit::OpenAIEdit(const std::string &response, const double total_t) :
    OpenAIResponse(response), total_time(total_t)
{
    this->input_tokens = this->response_["usage"]["input_tokens"];
    this->output_tokens = this->response_["usage"]["output_tokens"];

    this->unpack_structured_output_();
}

void OpenAIEdit::unpack_structured_output_()
{
    nlohmann::json structured_output;

    try {
        const std::string text = this->get_text_from_response_();
        structured_output = nlohmann::json::parse(text);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse structured output: {}", e.what()));
    }

    this->code = structured_output["code"];
    this->description_of_changes = structured_output["description_of_changes"];
}

} // namespace adapters
