#include "curl_base.hpp"
#include "configs.hpp"

#include <cstdlib>
#include <fmt/core.h>
#include <json.hpp>

namespace {

std::string get_openai_user_api_key()
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

size_t write_callback(char *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

inline nlohmann::json get_schema()
{
    // template seems to be consistent between Ollama and OpenAI
    return {
        { "type", "object" },
        {
            "properties",
            {
                { "code", { { "type", "string" } } },
                { "description_of_changes", { { "type", "string" } } },
            },
        },
        { "required", { "code", "description_of_changes" } },
        { "additionalProperties", false }, // not sure if ollama needs this
    };
}

nlohmann::json post_data_openai(const std::string &prompt)
{
    const nlohmann::json response_format = {
        {
            "format",
            {
                { "name", "updated_code" },
                { "schema", get_schema() },
                { "strict", true },
                { "type", "json_schema" },
            },
        }
    };
    return {
        { "input", prompt },
        { "model", configs.model_openai },
        { "store", false },
        { "temperature", 1.00 },
        { "text", response_format },
    };
}

nlohmann::json post_data_ollama(const std::string &prompt)
{
    return {
        { "format", get_schema() },
        { "model", configs.model_ollama },
        { "prompt", prompt },
        { "stream", false },
    };
}

} // namespace

namespace curl_base {

Curl::Curl()
{
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        throw std::runtime_error("Something went wrong when initializing libcurl");
    }

    this->handle_ = curl_easy_init();

    if (this->handle_ == nullptr) {
        throw std::runtime_error("Something went wrong when starting libcurl easy session");
    }
}

Curl::~Curl()
{
    if (this->handle_) {
        curl_slist_free_all(this->headers_);
        curl_easy_cleanup(this->handle_);
    }

    curl_global_cleanup();
}

void Curl::reset_handle_and_headers_()
{
    if (this->handle_) {
        curl_easy_reset(this->handle_);
    }

    curl_slist_free_all(this->headers_);
    this->headers_ = nullptr;
}

CurlResult Curl::create_openai_response(const std::string &prompt)
{
    this->reset_handle_and_headers_();

    const std::string header_auth = "Authorization: Bearer " + get_openai_user_api_key();
    this->headers_ = curl_slist_append(this->headers_, header_auth.c_str());

    const std::string header_content_type = "Content-Type: application/json";
    this->headers_ = curl_slist_append(this->headers_, header_content_type.c_str());

    curl_easy_setopt(this->handle_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(this->handle_, CURLOPT_HTTPHEADER, this->headers_);

    static std::string url_openai_responses = "https://api.openai.com/v1/responses";
    curl_easy_setopt(this->handle_, CURLOPT_URL, url_openai_responses.c_str());
    curl_easy_setopt(this->handle_, CURLOPT_POST, 1L);

    const nlohmann::json post_data = post_data_openai(prompt);
    const std::string post_data_str = post_data.dump();
    curl_easy_setopt(this->handle_, CURLOPT_POSTFIELDS, post_data_str.c_str());

    std::string response;
    curl_easy_setopt(this->handle_, CURLOPT_WRITEDATA, &response);

    const CURLcode code = curl_easy_perform(this->handle_);
    return check_curl_code(this->handle_, code, response);
}

CurlResult Curl::create_ollama_response(const std::string &prompt)
{
    this->reset_handle_and_headers_();

    const std::string header_content_type = "Content-Type: application/json";
    this->headers_ = curl_slist_append(this->headers_, header_content_type.c_str());

    curl_easy_setopt(this->handle_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(this->handle_, CURLOPT_HTTPHEADER, this->headers_);

    static std::string url_ollama_generate = fmt::format("http://{}:{}/api/generate", configs.host_ollama, configs.port_ollama);
    curl_easy_setopt(this->handle_, CURLOPT_URL, url_ollama_generate.c_str());
    curl_easy_setopt(this->handle_, CURLOPT_POST, 1L);

    const nlohmann::json post_data = post_data_ollama(prompt);
    const std::string post_data_str = post_data.dump();
    curl_easy_setopt(this->handle_, CURLOPT_POSTFIELDS, post_data_str.c_str());

    std::string response;
    curl_easy_setopt(this->handle_, CURLOPT_WRITEDATA, &response);

    const CURLcode code = curl_easy_perform(this->handle_);
    return check_curl_code(this->handle_, code, response);
}

} // namespace curl_base
