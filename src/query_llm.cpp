#include "query_llm.hpp"

#include "curl_base.hpp"
#include "utils.hpp"

#include <fmt/core.h>
#include <json.hpp>

namespace {

std::string get_stringified_json_from_output(const std::string &output)
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

    if (output.empty()) {
        throw std::runtime_error("Output from OpenAI is empty. Cannot extract JSON");
    }

    if (output[0] == '{' and output.back() == '}') {
        return output;
    }

    bool append_enabled = false;
    std::string line;
    std::stringstream ss(output);
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

    query_llm::LLMResponse results;
    std::string output;

    if (content["type"] == "output_text") {
        output = content["text"];
    } else if (content["type"] == "refusal") {
        throw std::runtime_error(fmt::format("OpenAI returned a refusal: {}", content["refusal"]));
    } else {
        throw std::runtime_error("Some unknown object type was returned from OpenAI");
    }

    const std::string raw_json = get_stringified_json_from_output(output);
    const nlohmann::json json_content = utils::parse_json(raw_json);

    results.output_tokens = json["usage"]["output_tokens"];
    results.description = json_content["description"];
    results.output_text = json_content["code"];
    results.input_tokens = json["usage"]["input_tokens"];
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

    const std::string raw_response = json["response"];
    const nlohmann::json json_content = utils::parse_json(raw_response);

    query_llm::LLMResponse results;
    results.input_tokens = json["prompt_eval_count"];
    results.output_tokens = json["eval_count"];
    results.description = json_content["description_of_changes"];
    results.output_text = json_content["code"];
    return results;
}

} // namespace

namespace query_llm {

LLMResponse run_openai_query(const std::string &prompt, const std::string &model)
{
    curl_base::Curl curl;
    const auto result = curl.create_openai_response(prompt, model);

    if (not result) {
        deserialize_openai_response_and_throw_error(result.error().response);
    }

    return deserialize_openai_response(result->response);
}

LLMResponse run_ollama_query(const std::string &prompt, const std::string &model)
{
    curl_base::Curl curl;
    const auto result = curl.create_ollama_response(prompt, model);

    if (not result) {
        deserialize_ollama_response_and_throw_error(result.error().response);
    }

    return deserialize_ollama_response(result->response);
}

} // namespace query_llm
