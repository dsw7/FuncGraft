#include "query_openai.hpp"

#include "curl_base.hpp"

#include <json.hpp>

namespace {

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

    curl_base::Curl curl;
    const std::string response = curl.create_chat_completion(request);
    return response;
}

} // namespace query_openai
