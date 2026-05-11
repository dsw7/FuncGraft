#include "responses_ollama.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace adapters {

OllamaError::OllamaError(const std::string &response, const int status_code) :
    ErrorResponse(response, status_code)
{
    if (not this->json_.contains("error")) {
        throw std::runtime_error("An error occurred but 'error' key not found in the response JSON");
    }

    this->errmsg = this->json_["error"];
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

OllamaEdit::OllamaEdit(const std::string &response, const double total_t) :
    OllamaResponse(response), total_time(total_t)
{
    this->input_tokens = this->response_["prompt_eval_count"];
    this->output_tokens = this->response_["eval_count"];

    this->unpack_structured_output_();
}

void OllamaEdit::unpack_structured_output_()
{
    nlohmann::json structured_output;

    try {
        const std::string content = this->response_["message"]["content"];
        structured_output = nlohmann::json::parse(content);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse structured output: {}", e.what()));
    }

    this->code = structured_output["code"];
    this->description_of_changes = structured_output["description_of_changes"];
}

} // namespace adapters
