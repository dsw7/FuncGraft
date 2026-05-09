#include "structured_output.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace {

nlohmann::json parse_structured_output_(const std::string &content)
{
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(content);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse structured output: {}", e.what()));
    }

    return json;
}

} // namespace

namespace structured_output {

nlohmann::json schema_classify_instructions()
{
    return {
        { "type", "object" },
        {
            "properties",
            {
                { "valid_instructions", { { "type", "boolean" } } },
                { "reasoning", { { "type", "string" } } },
            },
        },
        { "required", { "valid_instructions", "reasoning" } },
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

SchemaClassifyInstructions::SchemaClassifyInstructions(const std::string &content)
{
    const nlohmann::json json = parse_structured_output_(content);

    this->valid_instructions = json.at("valid_instructions").get<bool>();
    this->reasoning = json["reasoning"];
}

SchemaEditCode::SchemaEditCode(const std::string &content)
{
    const nlohmann::json json = parse_structured_output_(content);

    this->was_refused = json.at("was_refused").get<bool>();
    this->code = json["code"];
    this->description = json["description_of_changes"];
}

} // namespace structured_output
