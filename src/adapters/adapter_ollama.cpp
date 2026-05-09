#include "adapter_ollama.hpp"

#include "structured_output.hpp"
#include "system_prompts.hpp"

#include <fmt/core.h>
#include <json.hpp>
#include <stdexcept>

namespace adapters {

OllamaEditResponse::OllamaEditResponse(const std::string &response, const double total_time)
{
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse response: {}", e.what()));
    }

    if (not json.contains("done")) {
        throw std::runtime_error("The response from Ollama does not contain the 'done' key");
    }

    if (not json["done"]) {
        throw std::runtime_error("The response from Ollama indicates the job is not done");
    }

    const structured_output::SchemaEditCode so(json["message"]["content"]);
    this->description = so.description;
    this->output_text = so.code;
    this->was_refused = so.was_refused;

    this->input_tokens = json["prompt_eval_count"];
    this->output_tokens = json["eval_count"];
    this->total_time = total_time;
}

OllamaError::OllamaError(const std::string &response, const int status_code) :
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

    this->errmsg = json["error"];
}

Ollama::Ollama(const Configurations &configs)
{
    this->model_ = configs.model_ollama;
    this->host_ollama_ = configs.host_ollama;
    this->port_ollama_ = configs.port_ollama;
}

std::string Ollama::query_chat_api_(const std::string &post_fields)
{
    const std::string url = fmt::format("http://{}:{}/api/chat", this->host_ollama_, this->port_ollama_);
    curl_easy_setopt(this->handle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(this->handle_, CURLOPT_POST, 1L);

    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
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

std::expected<OllamaEditResponse, OllamaError> Ollama::query_edit_code(const std::string &prompt)
{
    const auto messages = nlohmann::json::array({
        { { "role", "system" }, { "content", system_prompts::system_prompt_edit_code() } },
        { { "role", "user" }, { "content", prompt } },
    });
    const nlohmann::json fields = {
        { "format", structured_output::schema_edit_code() },
        { "messages", messages },
        { "model", this->model_ },
        { "stream", false },
    };

    const std::string post_fields = fields.dump();
    const std::string response = this->query_chat_api_(post_fields);

    long http_status_code = this->get_http_status_code_();
    if (http_status_code != 200) {
        return std::unexpected(OllamaError(response, http_status_code));
    }

    const double total_time = this->get_rtt_time_();
    return OllamaEditResponse(response, total_time);
}

} // namespace adapters
