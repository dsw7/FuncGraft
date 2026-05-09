#pragma once

#include "configs.hpp"
#include "curl_base.hpp"

#include <expected>
#include <json.hpp>
#include <string>

namespace adapters {

class OllamaResponse {
public:
    OllamaResponse() = default;
    OllamaResponse(const std::string &response);

protected:
    nlohmann::json response_;
};

class OllamaClassificationResponse: public OllamaResponse {
public:
    OllamaClassificationResponse(const std::string &response);

    bool valid_instructions = false;
    std::string reasoning;

private:
    void unpack_structured_output_();
};

class OllamaEditResponse: public OllamaResponse {
public:
    OllamaEditResponse() :
        OllamaResponse() {} // needed for variants
    OllamaEditResponse(const std::string &response, const double total_t);

    bool was_refused = false;
    std::string description;
    std::string output_text;

    double total_time = 0.0;
    int input_tokens = 0;
    int output_tokens = 0;

private:
    void unpack_structured_output_();
};

struct OllamaError {
    OllamaError(const std::string &response, const int status_code);

    std::string errmsg;
    int status_code = 200;
};

class Ollama: public CurlBase {
public:
    Ollama(const Configurations &configs);
    std::expected<OllamaClassificationResponse, OllamaError> classify_instructions(const std::string &instructions);
    std::expected<OllamaEditResponse, OllamaError> query_edit_code(const std::string &prompt);

private:
    std::string query_chat_api_(const std::string &post_fields);

    std::string model_;
    std::string host_ollama_;
    int port_ollama_ = 11434;
};

} // namespace adapters
