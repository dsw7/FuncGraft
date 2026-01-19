#include "query_openai.hpp"

#include "curl_base.hpp"
#include "utils.hpp"

#include <fmt/core.h>
#include <json.hpp>
#include <stdexcept>

namespace {

void deserialize_openai_response_and_throw_error_(const std::string &response)
{
    const nlohmann::json json = utils::parse_json(response);

    if (not json.contains("error")) {
        throw std::runtime_error("An error occurred but 'error' key not found in the response JSON");
    }

    if (not json["error"].contains("message")) {
        throw std::runtime_error("An error occurred but 'error.message' not found in the response JSON");
    }

    throw std::runtime_error(json["error"]["message"]);
}

query_llm::LLMResponse deserialize_openai_response_(const std::string &response)
{
    const nlohmann::json json = utils::parse_json(response);

    if (json.contains("object")) {
        if (json["object"] != "response") {
            throw std::runtime_error("The response from OpenAI is not an OpenAI Response");
        }
    } else {
        throw std::runtime_error("The response from OpenAI does not contain an 'object' key");
    }

    nlohmann::json content;
    bool job_complete = false;

    for (const auto &item: json["output"]) {
        if (item["type"] != "message") {
            continue;
        }

        if (item["status"] == "completed") {
            content = item["content"][0];
            job_complete = true;
            break;
        }
    }

    if (not job_complete) {
        throw std::runtime_error("OpenAI did not complete the transaction");
    }

    std::string output;

    if (content["type"] == "output_text") {
        output = content["text"];
    } else if (content["type"] == "refusal") {
        throw std::runtime_error(fmt::format("OpenAI returned a refusal: {}", content["refusal"]));
    } else {
        throw std::runtime_error("Some unknown object type was returned from OpenAI");
    }

    const auto &[code, description] = query_llm::deserialize_structured_output(output);

    query_llm::LLMResponse results;
    results.description = description;
    results.input_tokens = json["usage"]["input_tokens"];
    results.output_text = code;
    results.output_tokens = json["usage"]["output_tokens"];
    return results;
}

} // namespace

namespace query_llm {

LLMResponse run_openai_query(const std::string &prompt)
{
    curl_base::Curl curl;
    const auto result = curl.create_openai_response(prompt);

    if (not result) {
        deserialize_openai_response_and_throw_error_(result.error().response);
    }

    return deserialize_openai_response_(result->response);
}

} // namespace query_llm
