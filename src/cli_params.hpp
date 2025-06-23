#pragma once

#include <filesystem>
#include <string>

namespace cli_params {

struct Parameters {
    std::string model;
    std::filesystem::path output_file;
    std::filesystem::path instructions_file;
    std::string rule;
    std::filesystem::path input_file;
};

} // namespace cli_params
