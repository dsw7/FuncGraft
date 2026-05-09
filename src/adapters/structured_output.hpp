#pragma once

#include <json.hpp>

namespace structured_output {
nlohmann::json schema_classify_instructions();
nlohmann::json schema_edit_code();
} // namespace structured_output
