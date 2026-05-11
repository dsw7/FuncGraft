#include "adapter_openai.hpp"

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

} // namespace adapters
