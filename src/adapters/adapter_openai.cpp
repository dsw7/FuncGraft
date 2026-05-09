#include "adapter_openai.hpp"

#include "structured_output.hpp"
#include "system_prompts.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace {

std::string get_openai_user_api_key_()
{
    static std::string api_key;

    if (api_key.empty()) {
        const char *env_api_key = std::getenv("OPENAI_API_KEY");

        if (env_api_key == nullptr) {
            throw std::runtime_error("OPENAI_API_KEY environment variable not set");
        }

        api_key = env_api_key;
    }

    return api_key;
}

} // namespace

namespace adapters {

OpenAIResponse::OpenAIResponse(const std::string &response)
{
    try {
        this->response_ = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse response: {}", e.what()));
    }

    if (not this->response_.contains("object")) {
        throw std::runtime_error("The response from OpenAI does not contain an 'object' key");
    }

    if (this->response_["object"] != "response") {
        throw std::runtime_error("The response from OpenAI is not an OpenAI Response");
    }
}

OpenAIEditResponse::OpenAIEditResponse(const std::string &response, const double total_t) :
    OpenAIResponse(response), total_time(total_t)
{
    this->input_tokens = this->response_["usage"]["input_tokens"];
    this->output_tokens = this->response_["usage"]["output_tokens"];

    this->unpack_structured_output_();
}

void OpenAIEditResponse::unpack_structured_output_()
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

    nlohmann::json structured_output;

    try {
        structured_output = nlohmann::json::parse(text);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse structured output: {}", e.what()));
    }

    this->was_refused = structured_output.at("was_refused").get<bool>();
    this->output_text = structured_output["code"];
    this->description = structured_output["description_of_changes"];
}

OpenAIError::OpenAIError(const std::string &response, const int status_code) :
    status_code(status_code)
{
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse response: {}", e.what()));
    }

    if (not json.contains("error")) {
        throw std::runtime_error("An error occurred but 'error' key not found in the response JSON");
    }

    if (not json["error"].contains("message")) {
        throw std::runtime_error("An error occurred but 'error.message' not found in the response JSON");
    }

    this->errmsg = json["error"]["message"];
}

OpenAI::OpenAI(const Configurations &configs)
{
    this->model_ = configs.model_openai;
}

std::string OpenAI::query_responses_api_(const std::string &post_fields)
{
    curl_easy_setopt(this->handle_, CURLOPT_URL, "https://api.openai.com/v1/responses");
    curl_easy_setopt(this->handle_, CURLOPT_POST, 1L);

    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + get_openai_user_api_key_()).c_str());
    curl_easy_setopt(this->handle_, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(this->handle_, CURLOPT_POSTFIELDS, post_fields.c_str());

    std::string response;
    curl_easy_setopt(this->handle_, CURLOPT_WRITEDATA, &response);

    const CURLcode code = curl_easy_perform(this->handle_);
    curl_slist_free_all(headers);
    headers = nullptr;

    if (code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
    }

    return response;
}

std::expected<OpenAIEditResponse, OpenAIError> OpenAI::query_edit_code(const std::string &prompt)
{
    const nlohmann::json response_format = {
        {
            "format",
            {
                { "name", "updated_code" },
                { "schema", structured_output::schema_edit_code() },
                { "strict", true },
                { "type", "json_schema" },
            },
        }
    };
    const nlohmann::json fields = {
        { "input", prompt },
        { "instructions", system_prompts::system_prompt_edit_code() },
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
    return OpenAIEditResponse(response, total_time);
}

} // namespace adapters
