#include "adapter_ollama.hpp"

#include <fmt/core.h>
#include <json.hpp>
#include <stdexcept>

namespace {

nlohmann::json get_structured_output_schema_()
{
    return {
        { "type", "object" },
        {
            "properties",
            {
                { "was_refused", { { "type", "boolean" } } },
                { "code", { { "type", "string" } } },
                { "description_of_changes", { { "type", "string" } } },
            },
        },
        { "required", { "was_refused", "code", "description_of_changes" } },
    };
}

nlohmann::json build_conversation_(const std::string &prompt)
{
    return nlohmann::json::array({
        { { "role", "system" }, { "content", R"(
You are a helpful assistant that specializes in programming.
The user will provide some code and instructions on what to do with the the code.

If the query makes sense and is related to programming, then:
  Set `was_refused` to false.
  Set `code` to your updated code.
  Set `description_of_changes` to a summary of the changes you applied.
Otherwise:
  Set `was_refused` to true.
  Set `code` to an empty string.
  Set `description_of_changes` to a summary of why you refused to process the query.
)" } },
        { { "role", "user" }, { "content", prompt } },
    });
}

std::string get_post_fields_(const std::string &prompt, const std::string &model)
{
    const nlohmann::json fields = {
        { "format", get_structured_output_schema_() },
        { "messages", build_conversation_(prompt) },
        { "model", model },
        { "stream", false },
    };

    return fields.dump();
}

struct StructuredOutput_ {
    StructuredOutput_(const std::string &message);

    bool was_refused = false;
    std::string code;
    std::string description;
};

StructuredOutput_::StructuredOutput_(const std::string &content)
{
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(content);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse structured output: {}", e.what()));
    }

    this->was_refused = json.at("was_refused").get<bool>();
    this->code = json["code"];
    this->description = json["description_of_changes"];
}

} // namespace

namespace adapters {

OllamaResponse::OllamaResponse(const std::string &response)
{
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse response: {}", e.what()));
    }

    if (not json.contains("done")) {
        throw std::runtime_error("The response from Ollama does not contain the 'done' key");
    }

    if (not json["done"]) {
        throw std::runtime_error("The response from Ollama indicates the job is not done");
    }

    const StructuredOutput_ structured_output(json["message"]["content"]);
    this->description = structured_output.description;
    this->output_text = structured_output.code;
    this->was_refused = structured_output.was_refused;

    this->input_tokens = json["prompt_eval_count"];
    this->output_tokens = json["eval_count"];
}

OllamaError::OllamaError(const std::string &response, const int status_code) :
    status_code(status_code)
{
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse response: {}", e.what()));
    }

    if (not json.contains("error")) {
        throw std::runtime_error("An error occurred but 'error' key not found in the response JSON");
    }

    this->errmsg = json["error"];
}

Ollama::Ollama(const Configurations &configs)
{
    this->model_ = configs.model_ollama;
    this->host_ollama_ = configs.host_ollama;
    this->port_ollama_ = configs.port_ollama;
}

std::expected<OllamaResponse, OllamaError> Ollama::query_generate_api(const std::string &prompt)
{
    const std::string url = fmt::format("http://{}:{}/api/chat", this->host_ollama_, this->port_ollama_);
    curl_easy_setopt(this->handle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(this->handle_, CURLOPT_POST, 1L);

    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(this->handle_, CURLOPT_HTTPHEADER, headers);

    const std::string post_fields = get_post_fields_(prompt, this->model_);
    curl_easy_setopt(this->handle_, CURLOPT_POSTFIELDS, post_fields.c_str());

    std::string response;
    curl_easy_setopt(this->handle_, CURLOPT_WRITEDATA, &response);

    const CURLcode code = curl_easy_perform(this->handle_);
    curl_slist_free_all(headers);
    headers = nullptr;

    if (code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
    }

    long http_status_code = -1;
    curl_easy_getinfo(this->handle_, CURLINFO_RESPONSE_CODE, &http_status_code);

    if (http_status_code != 200) {
        return std::unexpected(OllamaError(response, http_status_code));
    }

    return OllamaResponse(response);
}

} // namespace adapters
