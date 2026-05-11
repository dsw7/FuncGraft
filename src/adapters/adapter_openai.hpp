#pragma once

#include "configs.hpp"
#include "curl_base.hpp"
#include "responses_openai.hpp"

#include <expected>
#include <string>

namespace adapters {

class OpenAI: public CurlBase {
public:
    OpenAI(const Configurations &configs);
    std::expected<OpenAIEdit, OpenAIError> query_edit_code(const std::string &prompt);

protected:
    std::string query_responses_api_(const std::string &post_fields);

    std::string model_;
};

} // namespace adapters
