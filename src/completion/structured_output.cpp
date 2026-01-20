#include "structured_output.hpp"

#include "utils.hpp"

#include <json.hpp>
#include <stdexcept>

namespace completion {

StructuredOutput deserialize_structured_output(const std::string &output)
{
    const nlohmann::json json = utils::parse_json(output);

    if (not json.contains("code")) {
        throw std::runtime_error("Structured output missing 'code' field");
    }

    if (not json.contains("description_of_changes")) {
        throw std::runtime_error("Structured output missing 'description_of_changes' field");
    }

    return {
        json["code"],
        json["description_of_changes"],
    };
}

} // namespace completion
