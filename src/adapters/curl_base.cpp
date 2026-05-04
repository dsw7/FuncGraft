#include "curl_base.hpp"

#include <fmt/core.h>
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

double CurlBase::get_rtt_time_()
{
    double rtt_time = 0.0;

    const CURLcode return_code = curl_easy_getinfo(this->handle_, CURLINFO_TOTAL_TIME, &rtt_time);
    if (return_code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(return_code));
    }

    return rtt_time;
}

} // namespace adapters
