#pragma once

#include <curl/curl.h>
#include <string>

namespace curl_base {

class Curl {
public:
    Curl();
    ~Curl();

    // We want to prevent any copies from being made otherwise we'll attempt
    // to delete a shallow copy of the headers list multiple times (i.e. because the destructor will
    // be called for each copy)
    Curl(const Curl &) = delete;
    Curl &operator=(const Curl &) = delete;
    std::string create_chat_completion(const std::string &request);

private:
    CURL *handle_ = nullptr;
    struct curl_slist *headers_ = nullptr;
};

} // namespace curl_base
