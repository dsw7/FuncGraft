#include "adapter_ollama.hpp"

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

} // namespace adapters
