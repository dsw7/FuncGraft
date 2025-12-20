#include "query_llm.hpp"

#include "curl_base.hpp"
#include "utils.hpp"

#include <fmt/core.h>
#include <json.hpp>

namespace {

void deserialize_openai_response_and_throw_error(const std::string &response)
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

void deserialize_ollama_response_and_throw_error(const std::string &response)
{
    const nlohmann::json json = utils::parse_json(response);

    if (not json.contains("error")) {
        throw std::runtime_error("An error occurred but 'error' key not found in the response JSON");
    }

    throw std::runtime_error(json["error"]);
}

struct StructuredOutput {
    std::string code;
    std::string description_of_changes;
};

StructuredOutput deserialize_structured_output(const std::string &output)
{
    const nlohmann::json json = utils::parse_json(output);

    if (not json.contains("code")) {
        throw std::runtime_error("Structured output missing 'code' field");
    }

    if (not json.contains("description_of_changes")) {
        throw std::runtime_error("Structured output missing 'description_of_changes' field");
    }

    return {
        json["code"],
        json["description_of_changes"],
    };
}

query_llm::LLMResponse deserialize_openai_response(const std::string &response)
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

    const StructuredOutput so = deserialize_structured_output(output);

    query_llm::LLMResponse results;
    results.description = so.description_of_changes;
    results.input_tokens = json["usage"]["input_tokens"];
    results.output_text = so.code;
    results.output_tokens = json["usage"]["output_tokens"];
    return results;
}

query_llm::LLMResponse deserialize_ollama_response(const std::string &response)
{
    const nlohmann::json json = utils::parse_json(response);

    if (not json.contains("done")) {
        throw std::runtime_error("The response from Ollama does not contain the 'done' key");
    }

    if (not json["done"]) {
        throw std::runtime_error("The response from Ollama indicates the job is not done");
    }

    const std::string output = json["response"];
    const StructuredOutput so = deserialize_structured_output(output);

    query_llm::LLMResponse results;
    results.description = so.description_of_changes;
    results.input_tokens = json["prompt_eval_count"];
    results.output_text = so.code;
    results.output_tokens = json["eval_count"];
    return results;
}

} // namespace

namespace query_llm {

LLMResponse run_openai_query(const std::string &prompt)
{
    curl_base::Curl curl;
    const auto result = curl.create_openai_response(prompt);

    if (not result) {
        deserialize_openai_response_and_throw_error(result.error().response);
    }

    return deserialize_openai_response(result->response);
}

LLMResponse run_ollama_query(const std::string &prompt)
{
    curl_base::Curl curl;
    const auto result = curl.create_ollama_response(prompt);

    if (not result) {
        deserialize_ollama_response_and_throw_error(result.error().response);
    }

    return deserialize_ollama_response(result->response);
}

} // namespace query_llm
