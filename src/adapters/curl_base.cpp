#include "curl_base.hpp"

#include "configs.hpp"

#include <fmt/core.h>
#include <json.hpp>
#include <stdexcept>
#include <string>

namespace {

size_t write_callback(char *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

} // namespace

namespace adapters {

CurlBase::CurlBase()
{
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        throw std::runtime_error("Something went wrong when initializing libcurl");
    }

    this->handle_ = curl_easy_init();

    if (this->handle_ == nullptr) {
        throw std::runtime_error("Something went wrong when starting libcurl easy session");
    }

    curl_easy_setopt(this->handle_, CURLOPT_WRITEFUNCTION, write_callback);
}

CurlBase::~CurlBase()
{
    if (this->handle_) {
        curl_easy_cleanup(this->handle_);
    }

    curl_global_cleanup();
}

} // namespace adapters
