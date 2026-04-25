#include "adapter_ollama.hpp"

#include "components.hpp"

#include <fmt/core.h>
#include <json.hpp>
#include <stdexcept>

namespace {

nlohmann::json build_conversation_(const std::string &prompt)
{
    return nlohmann::json::array({
        { { "role", "system" }, { "content", components::get_system_prompt() } },
        { { "role", "user" }, { "content", prompt } },
    });
}

std::string get_post_fields_(const std::string &prompt, const std::string &model)
{
    const nlohmann::json fields = {
        { "format", components::get_structured_output_schema() },
        { "messages", build_conversation_(prompt) },
        { "model", model },
        { "stream", false },
    };

    return fields.dump();
}

} // namespace

namespace adapters {

OllamaResponse::OllamaResponse(const std::string &response)
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

    const components::StructuredOutput so(json["message"]["content"]);
    this->description = so.description;
    this->output_text = so.code;
    this->was_refused = so.was_refused;

    this->input_tokens = json["prompt_eval_count"];
    this->output_tokens = json["eval_count"];
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

std::expected<OllamaResponse, OllamaError> Ollama::query_generate_api(const std::string &prompt)
{
    const std::string url = fmt::format("http://{}:{}/api/chat", this->host_ollama_, this->port_ollama_);
    curl_easy_setopt(this->handle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(this->handle_, CURLOPT_POST, 1L);

    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
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
    curl_easy_getinfo(this->handle_, CURLINFO_RESPONSE_CODE, &http_status_code);

    if (http_status_code != 200) {
        return std::unexpected(OllamaError(response, http_status_code));
    }

    return OllamaResponse(response);
}

} // namespace adapters
