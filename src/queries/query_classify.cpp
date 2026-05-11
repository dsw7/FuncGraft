#include "query_classify.hpp"

#include "structured_output.hpp"
#include "system_prompts.hpp"

namespace queries {

std::expected<adapters::OllamaClassification, adapters::OllamaError> OllamaClassifier::classify_instructions(const std::string &prompt)
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
        return std::unexpected(adapters::OllamaError(response, http_status_code));
    }

    return adapters::OllamaClassification(response);
}

} // namespace queries
