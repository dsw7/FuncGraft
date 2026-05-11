#include "query_edit_code.hpp"

#include <json.hpp>

namespace {

std::string system_prompt_edit_code_()
{
    return R"(
You are a helpful assistant that specializes in programming.
The user will provide some code and instructions on what to do with the code.

IMPORTANT: Do not wrap your response in backticks (```). Output the code
directly without markdown code fences.

Output:
- description_of_changes: brief summary of the changes you applied
- code: your updated code
)";
}

nlohmann::json structured_output_edit_code_()
{
    return {
        { "type", "object" },
        {
            "properties",
            {
                { "description_of_changes", { { "type", "string" } } },
                { "code", { { "type", "string" } } },
            },
        },
        { "required", { "description_of_changes", "code" } },
        { "additionalProperties", false },
    };
}

} // namespace

namespace queries {

std::expected<adapters::OpenAIEdit, adapters::OpenAIError> OpenAICodeEditor::edit_code(const std::string &prompt)
{
    const nlohmann::json response_format = {
        {
            "format",
            {
                { "name", "updated_code" },
                { "schema", structured_output_edit_code_() },
                { "strict", true },
                { "type", "json_schema" },
            },
        }
    };

    const nlohmann::json fields = {
        { "input", prompt },
        { "instructions", system_prompt_edit_code_() },
        { "model", this->model_ },
        { "store", false },
        { "temperature", 1.00 },
        { "text", response_format },
    };

    const std::string response = this->query_responses_api_(fields.dump());

    long http_status_code = this->get_http_status_code_();
    if (http_status_code != 200) {
        return std::unexpected(adapters::OpenAIError(response, http_status_code));
    }

    const double total_time = this->get_rtt_time_();
    return adapters::OpenAIEdit(response, total_time);
}

std::expected<adapters::OllamaEdit, adapters::OllamaError> OllamaCodeEditor::edit_code(const std::string &prompt)
{
    const auto messages = nlohmann::json::array({
        { { "role", "system" }, { "content", system_prompt_edit_code_() } },
        { { "role", "user" }, { "content", prompt } },
    });

    const nlohmann::json fields = {
        { "format", structured_output_edit_code_() },
        { "messages", messages },
        { "model", this->model_ },
        { "stream", false },
    };

    const std::string response = this->query_chat_api_(fields.dump());

    long http_status_code = this->get_http_status_code_();
    if (http_status_code != 200) {
        return std::unexpected(adapters::OllamaError(response, http_status_code));
    }

    const double total_time = this->get_rtt_time_();
    return adapters::OllamaEdit(response, total_time);
}

} // namespace queries
