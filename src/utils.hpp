#pragma once

#include <json.hpp>
#include <string>

namespace utils {

void print_separator();
std::string read_from_file(const std::string &filename);
void write_to_file(const std::string &filename, const std::string &text);
nlohmann::json parse_json(const std::string &response);

} // namespace utils
