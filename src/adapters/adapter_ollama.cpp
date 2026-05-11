#include "adapter_ollama.hpp"

#include "structured_output.hpp"
#include "system_prompts.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace adapters {

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

std::expected<OllamaClassification, OllamaError> Ollama::classify_instructions(const std::string &prompt)
{
    const auto messages = nlohmann::json::array({
        { { "role", "system" }, { "content", system_prompts::system_prompt_classify_instructions() } },
        { { "role", "user" }, { "content", prompt } },
    });
    const nlohmann::json fields = {
        { "format", structured_output::schema_classify_instructions() },
        { "messages", messages },
        { "model", this->model_ },
        { "stream", false },
        { "temperature", 0.00 },
    };

    const std::string response = this->query_chat_api_(fields.dump());

    long http_status_code = this->get_http_status_code_();
    if (http_status_code != 200) {
        return std::unexpected(OllamaError(response, http_status_code));
    }
    return OllamaClassification(response);
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

    const std::string response = this->query_chat_api_(fields.dump());

    long http_status_code = this->get_http_status_code_();
    if (http_status_code != 200) {
        return std::unexpected(OllamaError(response, http_status_code));
    }

    const double total_time = this->get_rtt_time_();
    return OllamaEditResponse(response, total_time);
}

} // namespace adapters
