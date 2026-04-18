#pragma once

#include <curl/curl.h>

namespace completion {

class CurlBase {
public:
    CurlBase();
    ~CurlBase();

    CurlBase(const CurlBase &) = delete;
    CurlBase &operator=(const CurlBase &) = delete;

protected:
    CURL *handle_ = nullptr;
};

} // namespace completion
