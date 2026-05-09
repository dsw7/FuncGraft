#pragma once

#include <json.hpp>
#include <string>

namespace structured_output {

nlohmann::json schema_classify_instructions();
nlohmann::json schema_edit_code();

struct SchemaClassifyInstructions {
    SchemaClassifyInstructions(const std::string &content);

    bool valid_instructions = false;
    std::string reasoning;
};

struct SchemaEditCode {
    SchemaEditCode(const std::string &content);

    bool was_refused = false;
    std::string code;
    std::string description;
};

} // namespace structured_output
