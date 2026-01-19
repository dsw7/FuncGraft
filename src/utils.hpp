#pragma once

#include <json.hpp>
#include <optional>
#include <string>

namespace utils {

void print_separator();
std::string read_from_file(const std::string &filename);
void write_to_file(const std::string &filename, const std::string &text);
std::optional<std::string> resolve_label_from_extension(const std::string &extension);
nlohmann::json parse_json(const std::string &response);

} // namespace utils
