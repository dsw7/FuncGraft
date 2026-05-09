#pragma once

#include <json.hpp>
#include <string>

namespace structured_output {

nlohmann::json get_structured_output_schema();

struct StructuredOutput {
    StructuredOutput(const std::string &message);

    bool was_refused = false;
    std::string code;
    std::string description;
};

} // namespace structured_output
