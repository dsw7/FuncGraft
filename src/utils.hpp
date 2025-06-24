#pragma once

#include <optional>
#include <string>

namespace utils {

void print_separator();
std::string read_from_file(const std::string &filename);
void write_to_file(const std::string &filename, const std::string &text);
std::optional<std::string> resolve_label_from_extension(const std::string &extension);
std::string get_code_block(const std::string &body);
std::string get_code_block(const std::string &body, const std::string &label);

} // namespace utils
