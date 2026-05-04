#include "adapter_openai.hpp"

#include "components.hpp"

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

std::string get_post_fields_(const std::string &prompt, const std::string &model)
{
    const nlohmann::json response_format = {
        {
            "format",
            {
                { "name", "updated_code" },
                { "schema", components::get_structured_output_schema() },
                { "strict", true },
                { "type", "json_schema" },
            },
        }
    };

    const nlohmann::json fields = {
        { "input", prompt },
        { "instructions", components::get_system_prompt() },
        { "model", model },
        { "store", false },
        { "temperature", 1.00 },
        { "text", response_format },
    };
    return fields.dump();
}

} // namespace

namespace adapters {

OpenAIResponse::OpenAIResponse(const std::string &response, const double total_time)
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

    const std::string content = this->extract_output_from_response_();
    const components::StructuredOutput so(content);
    this->description = so.description;
    this->output_text = so.code;
    this->was_refused = so.was_refused;

    this->input_tokens = this->response_["usage"]["input_tokens"];
    this->output_tokens = this->response_["usage"]["output_tokens"];
    this->total_time = total_time;
}

std::string OpenAIResponse::extract_output_from_response_()
{
    nlohmann::json content;
    bool job_complete = false;

    for (const auto &item: this->response_["output"]) {
        if (item["type"] != "message") {
            continue;
        }

        if (item["status"] == "completed") {
            content = item["content"][0];
            job_complete = true;
            break;
        }
    }

    if (not job_complete) {
        throw std::runtime_error("OpenAI did not complete the transaction");
    }

    if (content["type"] == "output_text") {
        return content["text"];
    }

    if (content["type"] == "refusal") {
        throw std::runtime_error(fmt::format("OpenAI returned a refusal: {}", content["refusal"]));
    }

    throw std::runtime_error("Some unknown object type was returned from OpenAI");
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

std::expected<OpenAIResponse, OpenAIError> OpenAI::query_messages_api(const std::string &prompt)
{
    curl_easy_setopt(this->handle_, CURLOPT_URL, "https://api.openai.com/v1/responses");
    curl_easy_setopt(this->handle_, CURLOPT_POST, 1L);

    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + get_openai_user_api_key_()).c_str());
    curl_easy_setopt(this->handle_, CURLOPT_HTTPHEADER, headers);

    const std::string post_fields = get_post_fields_(prompt, this->model_);
    curl_easy_setopt(this->handle_, CURLOPT_POSTFIELDS, post_fields.c_str());

    std::string response;
    curl_easy_setopt(this->handle_, CURLOPT_WRITEDATA, &response);

    const CURLcode code = curl_easy_perform(this->handle_);
    curl_slist_free_all(headers);
    headers = nullptr;

    if (code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
    }

    long http_status_code = -1;

    const CURLcode return_code = curl_easy_getinfo(this->handle_, CURLINFO_RESPONSE_CODE, &http_status_code);
    if (return_code == CURLE_OK) {
        if (http_status_code != 200) {
            return std::unexpected(OpenAIError(response, http_status_code));
        }
    } else {
        throw std::runtime_error(curl_easy_strerror(return_code));
    }

    const double total_time = this->get_rtt_time_();
    return OpenAIResponse(response, total_time);
}

} // namespace adapters
