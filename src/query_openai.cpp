#include "query_openai.hpp"

#include "curl_base.hpp"

#include <fmt/core.h>
#include <json.hpp>

namespace {

std::string get_stringified_json_from_completion(const std::string &completion)
{
    /*
     * Some models return a JSON completion without triple backticks:
     * {
     *   k: v,
     * }
     * Others return a JSON completion with triple backticks:
     * ```json
     * {
     *   k: v,
     * }
     * ```
     */

    if (completion[0] == '{' and completion.back() == '}') {
        return completion;
    }

    bool append_enabled = false;
    std::string line;
    std::stringstream ss(completion);
    std::string raw_json;

    while (std::getline(ss, line)) {
        if (line == "```json") {
            append_enabled = true;
        } else if (line == "```") {
            append_enabled = false;
        } else {
            if (append_enabled) {
                raw_json += line;
            }
        }
    }

    if (append_enabled) {
        throw std::runtime_error("Closing triple backticks not found. Raw JSON might be malformed");
    }

    return raw_json;
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
    const std::string raw_json = get_stringified_json_from_completion(content);

    nlohmann::json json_content;

    try {
        json_content = nlohmann::json::parse(raw_json);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse content: {}", e.what()));
    }

    query_openai::QueryResults results;
    results.completion_tokens = json["usage"]["completion_tokens"];
    results.description = json_content["description"];
    results.output_text = json_content["code"];
    results.prompt_tokens = json["usage"]["prompt_tokens"];
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
