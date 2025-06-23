#include "query_openai.hpp"

#include "utils.hpp"

#include <curl/curl.h>
#include <json.hpp>

namespace {

const std::string URL_CHAT_COMPLETIONS = "https://api.openai.com/v1/chat/completions";

size_t write_callback(char *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

class CurlBase {
public:
    CurlBase();
    ~CurlBase();

    // We want to prevent any copies from being made otherwise we'll attempt
    // to delete a shallow copy of the headers list multiple times (i.e. because the destructor will
    // be called for each copy)
    CurlBase(const CurlBase &) = delete;
    CurlBase &operator=(const CurlBase &) = delete;
    std::string create_chat_completion(const std::string &request);

private:
    CURL *handle_ = nullptr;
    struct curl_slist *headers_ = nullptr;
};

CurlBase::CurlBase()
{
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        throw std::runtime_error("Something went wrong when initializing libcurl");
    }

    this->handle_ = curl_easy_init();

    if (this->handle_ == nullptr) {
        throw std::runtime_error("Something went wrong when starting libcurl easy session");
    }
}

CurlBase::~CurlBase()
{
    if (this->handle_) {
        curl_slist_free_all(this->headers_);
        curl_easy_cleanup(this->handle_);
    }

    curl_global_cleanup();
}

std::string CurlBase::create_chat_completion(const std::string &request)
{
    if (this->handle_) {
        curl_easy_reset(this->handle_);
    }

    curl_slist_free_all(this->headers_);
    this->headers_ = nullptr;

    const std::string header_auth = "Authorization: Bearer " + utils::get_user_api_key();
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

    return response;
}

std::string serialize_request(const std::string &prompt, const std::string &model)
{
    const nlohmann::json messages = { { "role", "developer" }, { "content", prompt } };
    const nlohmann::json data = {
        { "model", model },
        { "temperature", 1.00 },
        { "messages", nlohmann::json::array({ messages }) },
    };

    return data.dump();
}

} // namespace

namespace query_openai {

std::string run_query(const std::string &prompt)
{
    const std::string request = serialize_request(prompt, "abc");

    CurlBase curl;
    const std::string response = curl.create_chat_completion(request);
    return response;
}

} // namespace query_openai
