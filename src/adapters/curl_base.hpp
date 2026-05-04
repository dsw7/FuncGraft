#pragma once

#include <curl/curl.h>

namespace adapters {

class CurlBase {
public:
    CurlBase();
    ~CurlBase();

    CurlBase(const CurlBase &) = delete;
    CurlBase &operator=(const CurlBase &) = delete;

protected:
    double get_rtt_time_();

    CURL *handle_ = nullptr;
};

} // namespace adapters
