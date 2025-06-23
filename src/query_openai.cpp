#include "query_openai.hpp"

#include "curl_base.hpp"

#include <fmt/core.h>
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

query_openai::QueryResults deserialize_response(const std::string &response)
{
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse response: {}", e.what()));
    }

    if (json.contains("error")) {
        throw std::runtime_error(json["error"]["message"]);
    }

    if (json["object"] != "chat.completion") {
        throw std::runtime_error("The returned object is not a chat completion!");
    }

    const std::string content = json["choices"][0]["message"]["content"];
    nlohmann::json json_content;

    try {
        json_content = nlohmann::json::parse(content);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse content: {}", e.what()));
    }

    query_openai::QueryResults results;
    results.code = json_content["code"];
    results.description = json_content["description"];
    results.prompt_tokens = json["usage"]["prompt_tokens"];
    results.completion_tokens = json["usage"]["completion_tokens"];
    return results;
}

} // namespace

namespace query_openai {

QueryResults run_query(const std::string &prompt, const std::string &model)
{
    const std::string request = serialize_request(prompt, model);

    curl_base::Curl curl;
    const std::string response = curl.create_chat_completion(request);
    return deserialize_response(response);
}

} // namespace query_openai
