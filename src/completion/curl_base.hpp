#pragma once

#include <curl/curl.h>
#include <expected>
#include <stdexcept>
#include <string>

namespace curl_base {

struct Ok {
    long code = -1;
    std::string response;
};

struct Error {
    long code = -1;
    std::string response;
};

using CurlResult = std::expected<Ok, Error>;

inline CurlResult check_curl_code(CURL *handle, const CURLcode code, const std::string &response)
{
    if (code != CURLE_OK) {
        // rare but catch truly exceptional cases
        throw std::runtime_error(curl_easy_strerror(code));
    }

    long http_status_code = -1;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &http_status_code);

    if (http_status_code != 200) {
        return std::unexpected(Error { http_status_code, response });
    }

    return Ok { http_status_code, response };
}

class Curl {
public:
    Curl();
    ~Curl();

    // We want to prevent any copies from being made otherwise we'll attempt
    // to delete a shallow copy of the headers list multiple times (i.e. because the destructor will
    // be called for each copy)
    Curl(const Curl &) = delete;
    Curl &operator=(const Curl &) = delete;
    CurlResult create_openai_response(const std::string &prompt);
    CurlResult create_ollama_response(const std::string &prompt);

private:
    CURL *handle_ = nullptr;
    struct curl_slist *headers_ = nullptr;
    void reset_handle_and_headers_();
};

} // namespace curl_base
