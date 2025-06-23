#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace params {

struct CommandLineParameters {
    bool verbose = false;
    std::filesystem::path input_file;
    std::optional<std::filesystem::path> output_file;
    std::optional<std::filesystem::path> instructions_file;
    std::optional<std::string> model;
    std::optional<std::string> rule;
};

struct InternalParameters {
    std::string input_text;
    std::string instructions;
};

} // namespace params
