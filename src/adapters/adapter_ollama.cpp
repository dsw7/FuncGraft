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
                { "code", { { "type", "string" } } },
                { "description_of_changes", { { "type", "string" } } },
            },
        },
        { "required", { "code", "description_of_changes" } },
    };
}

std::string get_post_fields_(const std::string &prompt, const std::string &model)
{
    const nlohmann::json fields = {
        { "format", get_structured_output_schema_() },
        { "model", model },
        { "prompt", prompt },
        { "stream", false },
    };

    return fields.dump();
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

    const std::string structured_output_s = json["response"];
    nlohmann::json structured_output_json;

    try {
        structured_output_json = nlohmann::json::parse(structured_output_s);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse structured output: {}", e.what()));
    }

    this->description = structured_output_json["description_of_changes"];
    this->output_text = structured_output_json["code"];

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

Ollama::Ollama(const std::string &model, const std::string &host_ollama, const int port_ollama) :
    model_(model), host_ollama_(host_ollama), port_ollama_(port_ollama) {}

std::expected<OllamaResponse, OllamaError> Ollama::query_generate_api(const std::string &prompt)
{
    const std::string url = fmt::format("http://{}:{}/api/generate", this->host_ollama_, this->port_ollama_);
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
