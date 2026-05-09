#include "structured_output.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace structured_output {

nlohmann::json get_structured_output_schema()
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
        { "additionalProperties", false },
    };
}

StructuredOutput::StructuredOutput(const std::string &content)
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

} // namespace structured_output
