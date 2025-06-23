#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace cli_params {

struct Parameters {
    std::filesystem::path input_file;
    std::optional<std::filesystem::path> output_file;
    std::optional<std::filesystem::path> instructions_file;
    std::optional<std::string> model;
    std::optional<std::string> rule;
};

} // namespace cli_params
