#include "query_edit_code.hpp"

#include "structured_output.hpp"
#include "system_prompts.hpp"

#include <json.hpp>

namespace queries {

std::expected<adapters::OllamaEdit, adapters::OllamaError> OllamaCodeEditor::edit_code(const std::string &prompt)
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
        return std::unexpected(adapters::OllamaError(response, http_status_code));
    }

    const double total_time = this->get_rtt_time_();
    return adapters::OllamaEdit(response, total_time);
}

} // namespace queries
