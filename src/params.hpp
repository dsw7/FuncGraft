#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace params {

struct CommandLineParameters {
    bool verbose = false;
    bool use_local_llm = false;
    std::filesystem::path input_file;
    std::optional<std::filesystem::path> instructions_file;
    std::optional<std::filesystem::path> output_file;
    std::optional<std::string> instructions_from_cli;

    void validate_params();
};

} // namespace params
