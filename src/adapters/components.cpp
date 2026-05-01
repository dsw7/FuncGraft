#include "components.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace components {

std::string get_system_prompt()
{
    return R"(
You are a helpful assistant that specializes in programming.
The user will provide some code and instructions on what to do with the the code.

IMPORTANT: Do not wrap your response in backticks (```). Output the code
directly without markdown code fences.

If the query makes sense and is related to programming, then:
  Set `was_refused` to false.
  Set `code` to your updated code.
  Set `description_of_changes` to a summary of the changes you applied.
Otherwise:
  Set `was_refused` to true.
  Set `code` to an empty string.
  Set `description_of_changes` to a summary of why you refused to process the query.
)";
}

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

} // namespace components
