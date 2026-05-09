#include "structured_output.hpp"

namespace structured_output {

nlohmann::json schema_classify_instructions()
{
    return {
        { "type", "object" },
        {
            "properties",
            {
                { "reasoning", { { "type", "string" } } },
                { "valid_instructions", { { "type", "boolean" } } },
            },
        },
        { "required", { "reasoning", "valid_instructions" } },
        { "additionalProperties", false },
    };
}

nlohmann::json schema_edit_code()
{
    return {
        { "type", "object" },
        {
            "properties",
            {
                { "description_of_changes", { { "type", "string" } } },
                { "code", { { "type", "string" } } },
            },
        },
        { "required", { "description_of_changes", "code" } },
        { "additionalProperties", false },
    };
}

} // namespace structured_output
