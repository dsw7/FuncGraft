#include "components.hpp"

namespace components {

std::string get_system_prompt()
{
    return R"(
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

} // namespace components
