#include "adapter_ollama.hpp"

#include "structured_output.hpp"
#include "system_prompts.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace adapters {

OllamaResponse::OllamaResponse(const std::string &response)
{
    try {
        this->response_ = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse response: {}", e.what()));
    }

    if (not this->response_.contains("done")) {
        throw std::runtime_error("The response from Ollama does not contain the 'done' key");
    }

    if (not this->response_["done"]) {
        throw std::runtime_error("The response from Ollama indicates the job is not done");
    }
}

OllamaClassificationResponse::OllamaClassificationResponse(const std::string &response) :
    OllamaResponse(response)
{
    nlohmann::json structured_output;

    try {
        const std::string content = this->response_["message"]["content"];
        structured_output = nlohmann::json::parse(content);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse structured output: {}", e.what()));
    }

    this->valid_instructions = structured_output.at("valid_instructions").get<bool>();
    this->reasoning = structured_output["reasoning"];
}

OllamaEditResponse::OllamaEditResponse(const std::string &response, const double total_t) :
    OllamaResponse(response), total_time(total_t)
{
    this->input_tokens = this->response_["prompt_eval_count"];
    this->output_tokens = this->response_["eval_count"];

    this->unpack_structured_output_();
}

void OllamaEditResponse::unpack_structured_output_()
{
    nlohmann::json structured_output;

    try {
        const std::string content = this->response_["message"]["content"];
        structured_output = nlohmann::json::parse(content);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse structured output: {}", e.what()));
    }

    this->output_text = structured_output["code"];
    this->description = structured_output["description_of_changes"];
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

std::string Ollama::query_chat_api_(const std::string &post_fields)
{
    const std::string url = fmt::format("http://{}:{}/api/chat", this->host_ollama_, this->port_ollama_);
    curl_easy_setopt(this->handle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(this->handle_, CURLOPT_POST, 1L);

    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(this->handle_, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(this->handle_, CURLOPT_POSTFIELDS, post_fields.c_str());

    std::string response;
    curl_easy_setopt(this->handle_, CURLOPT_WRITEDATA, &response);

    const CURLcode code = curl_easy_perform(this->handle_);
    curl_slist_free_all(headers);
    headers = nullptr;

    if (code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
    }

    return response;
}

std::expected<OllamaClassificationResponse, OllamaError> Ollama::classify_instructions(const std::string &prompt)
{
    const auto messages = nlohmann::json::array({
        { { "role", "system" }, { "content", system_prompts::system_prompt_classify_instructions() } },
        { { "role", "user" }, { "content", prompt } },
    });
    const nlohmann::json fields = {
        { "format", structured_output::schema_classify_instructions() },
        { "messages", messages },
        { "model", this->model_ },
        { "stream", false },
        { "temperature", 0.00 },
    };

    const std::string response = this->query_chat_api_(fields.dump());

    long http_status_code = this->get_http_status_code_();
    if (http_status_code != 200) {
        return std::unexpected(OllamaError(response, http_status_code));
    }
    return OllamaClassificationResponse(response);
}

std::expected<OllamaEditResponse, OllamaError> Ollama::query_edit_code(const std::string &prompt)
{
    const auto messages = nlohmann::json::array({
        { { "role", "system" }, { "content", system_prompts::system_prompt_edit_code() } },
        { { "role", "user" }, { "content", prompt } },
    });
    const nlohmann::json fields = {
        { "format", structured_output::schema_edit_code() },
        { "messages", messages },
        { "model", this->model_ },
        { "stream", false },
    };

    const std::string response = this->query_chat_api_(fields.dump());

    long http_status_code = this->get_http_status_code_();
    if (http_status_code != 200) {
        return std::unexpected(OllamaError(response, http_status_code));
    }

    const double total_time = this->get_rtt_time_();
    return OllamaEditResponse(response, total_time);
}

} // namespace adapters
