#include "query_ollama.hpp"

#include "curl_base.hpp"
#include "utils.hpp"

#include <json.hpp>
#include <stdexcept>

namespace {

void deserialize_ollama_response_and_throw_error_(const std::string &response)
{
    const nlohmann::json json = utils::parse_json(response);

    if (not json.contains("error")) {
        throw std::runtime_error("An error occurred but 'error' key not found in the response JSON");
    }

    throw std::runtime_error(json["error"]);
}

completion::LLMResponse deserialize_ollama_response_(const std::string &response)
{
    const nlohmann::json json = utils::parse_json(response);

    if (not json.contains("done")) {
        throw std::runtime_error("The response from Ollama does not contain the 'done' key");
    }

    if (not json["done"]) {
        throw std::runtime_error("The response from Ollama indicates the job is not done");
    }

    const std::string output = json["response"];
    const auto &[code, description] = completion::deserialize_structured_output(output);

    completion::LLMResponse results;
    results.description = description;
    results.input_tokens = json["prompt_eval_count"];
    results.output_text = code;
    results.output_tokens = json["eval_count"];
    return results;
}

} // namespace

namespace completion {

LLMResponse run_ollama_query(const std::string &prompt)
{
    Curl curl;
    const auto result = curl.create_ollama_response(prompt);

    if (not result) {
        deserialize_ollama_response_and_throw_error_(result.error().response);
    }

    return deserialize_ollama_response_(result->response);
}

} // namespace completion
