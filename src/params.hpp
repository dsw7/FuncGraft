#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace {

const std::string DEFAULT_MODEL = "gpt-4o";

}

namespace params {

struct CommandLineParameters {
    bool verbose = false;
    std::filesystem::path input_file;
    std::optional<std::filesystem::path> instructions_file;
    std::optional<std::filesystem::path> output_file;
    std::optional<std::string> instructions_from_cli;
    std::string model = DEFAULT_MODEL;

    void validate_params();
};

} // namespace params
