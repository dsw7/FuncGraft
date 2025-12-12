#pragma once

#include <curl/curl.h>
#include <expected>
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

class Curl {
public:
    Curl();
    ~Curl();

    // We want to prevent any copies from being made otherwise we'll attempt
    // to delete a shallow copy of the headers list multiple times (i.e. because the destructor will
    // be called for each copy)
    Curl(const Curl &) = delete;
    Curl &operator=(const Curl &) = delete;
    CurlResult create_chat_completion(const std::string &request);

private:
    CURL *handle_ = nullptr;
    struct curl_slist *headers_ = nullptr;
};

} // namespace curl_base
