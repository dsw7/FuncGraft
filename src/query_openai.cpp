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
    CURL *handle = nullptr;
    struct curl_slist *headers = nullptr;

    void reset_easy_handle();
    void reset_headers_list();
    void set_auth_token();
    void set_writefunction();
    void set_content_type_transmit_json();
    void run_easy_perform();
};

CurlBase::CurlBase()
{
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        throw std::runtime_error("Something went wrong when initializing libcurl");
    }

    this->handle = curl_easy_init();

    if (this->handle == nullptr) {
        throw std::runtime_error("Something went wrong when starting libcurl easy session");
    }
}

CurlBase::~CurlBase()
{
    if (this->handle) {
        curl_slist_free_all(this->headers);
        curl_easy_cleanup(this->handle);
    }

    curl_global_cleanup();
}

void CurlBase::reset_easy_handle()
{
    if (this->handle) {
        curl_easy_reset(this->handle);
    }
}

void CurlBase::reset_headers_list()
{
    curl_slist_free_all(this->headers);
    this->headers = nullptr;
}

void CurlBase::set_writefunction()
{
    curl_easy_setopt(this->handle, CURLOPT_WRITEFUNCTION, write_callback);
}

void CurlBase::set_auth_token()
{
    const std::string token = utils::get_user_api_key();
    const std::string header = "Authorization: Bearer " + token;
    this->headers = curl_slist_append(this->headers, header.c_str());
}

void CurlBase::set_content_type_transmit_json()
{
    const std::string header = "Content-Type: application/json";
    this->headers = curl_slist_append(this->headers, header.c_str());
}

void CurlBase::run_easy_perform()
{
    const CURLcode code = curl_easy_perform(this->handle);

    if (code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
    }
}

std::string CurlBase::create_chat_completion(const std::string &request)
{
    this->reset_easy_handle();
    this->reset_headers_list();
    this->set_writefunction();
    this->set_auth_token();

    this->set_content_type_transmit_json();
    curl_easy_setopt(this->handle, CURLOPT_HTTPHEADER, this->headers);

    curl_easy_setopt(this->handle, CURLOPT_URL, URL_CHAT_COMPLETIONS.c_str());

    curl_easy_setopt(this->handle, CURLOPT_POST, 1L);
    curl_easy_setopt(this->handle, CURLOPT_POSTFIELDS, request.c_str());

    std::string response;
    curl_easy_setopt(this->handle, CURLOPT_WRITEDATA, &response);

    this->run_easy_perform();
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
