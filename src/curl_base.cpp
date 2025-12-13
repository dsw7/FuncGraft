#include "curl_base.hpp"

#include <cstdlib>
#include <stdexcept>

namespace {

const std::string URL_CHAT_COMPLETIONS = "https://api.openai.com/v1/responses";

std::string get_user_api_key()
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

CurlResult Curl::create_chat_completion(const std::string &request)
{
    if (this->handle_) {
        curl_easy_reset(this->handle_);
    }

    curl_slist_free_all(this->headers_);
    this->headers_ = nullptr;

    const std::string header_auth = "Authorization: Bearer " + get_user_api_key();
    this->headers_ = curl_slist_append(this->headers_, header_auth.c_str());

    const std::string header_content_type = "Content-Type: application/json";
    this->headers_ = curl_slist_append(this->headers_, header_content_type.c_str());

    curl_easy_setopt(this->handle_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(this->handle_, CURLOPT_HTTPHEADER, this->headers_);
    curl_easy_setopt(this->handle_, CURLOPT_URL, URL_CHAT_COMPLETIONS.c_str());
    curl_easy_setopt(this->handle_, CURLOPT_POST, 1L);
    curl_easy_setopt(this->handle_, CURLOPT_POSTFIELDS, request.c_str());

    std::string response;
    curl_easy_setopt(this->handle_, CURLOPT_WRITEDATA, &response);

    const CURLcode code = curl_easy_perform(this->handle_);
    if (code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
    }

    long http_status_code = -1;
    curl_easy_getinfo(this->handle_, CURLINFO_RESPONSE_CODE, &http_status_code);

    if (http_status_code == 200) {
        return Ok { http_status_code, response };
    }

    return std::unexpected(Error { http_status_code, response });
}

} // namespace curl_base
