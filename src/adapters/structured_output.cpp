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
                { "was_refused", { { "type", "boolean" } } },
                { "code", { { "type", "string" } } },
                { "description_of_changes", { { "type", "string" } } },
            },
        },
        { "required", { "was_refused", "code", "description_of_changes" } },
        { "additionalProperties", false },
    };
}

} // namespace structured_output
