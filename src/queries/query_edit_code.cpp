#include "query_edit_code.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <json.hpp>
#include <stdexcept>

namespace {

std::string get_code_block_(const std::string &body, const std::string &label)
{
    if (body.back() == '\n') {
        return fmt::format("```{}\n{}```", label, body);
    }

    return fmt::format("```{}\n{}\n```", label, body);
}

std::string user_prompt_edit_code_(
    const std::string &instructions, const std::string &code, const std::string &language)
{
    if (utils::is_text_empty(instructions)) {
        throw std::runtime_error("Instructions are empty!");
    }

    if (utils::is_text_empty(code)) {
        throw std::runtime_error("Body is empty. Cannot extract code block");
    }

    return fmt::format(R"(Apply the following instructions:
{}
To the following code:
{}
)",
        get_code_block_(instructions, "plaintext"),
        get_code_block_(code, language));
}

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

std::expected<OpenAIEdit, OpenAIError> OpenAICodeEditor::edit_code(
    const std::string &instructions, const std::string &code, const std::string &language)
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
        { "input", user_prompt_edit_code_(instructions, code, language) },
        { "instructions", system_prompt_edit_code_() },
        { "model", this->model_ },
        { "store", false },
        { "temperature", 1.00 },
        { "text", response_format },
    };

    const std::string response = this->query_responses_api_(fields.dump());

    long http_status_code = this->get_http_status_code_();
    if (http_status_code != 200) {
        return std::unexpected(OpenAIError(response, http_status_code));
    }

    const double total_time = this->get_rtt_time_();
    return OpenAIEdit(response, total_time);
}

std::expected<OllamaEdit, OllamaError> OllamaCodeEditor::edit_code(
    const std::string &instructions, const std::string &code, const std::string &language)
{
    const auto messages = nlohmann::json::array({
        { { "role", "system" }, { "content", system_prompt_edit_code_() } },
        { { "role", "user" }, { "content", user_prompt_edit_code_(instructions, code, language) } },
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
        return std::unexpected(OllamaError(response, http_status_code));
    }

    const double total_time = this->get_rtt_time_();
    return OllamaEdit(response, total_time);
}

} // namespace queries
