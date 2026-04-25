#pragma once

#include <json.hpp>
#include <string>

namespace components {

std::string get_system_prompt();
nlohmann::json get_structured_output_schema();

} // namespace components
