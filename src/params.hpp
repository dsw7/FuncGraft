#pragma once

#include <filesystem>
#include <optional>
#include <string>

struct Configs {
    void load_configs_from_config_file();

    std::string model_ollama;
    std::string model_openai;
};

extern Configs configs;

namespace params {

struct CommandLineParameters {
    bool verbose = false;
    bool use_local_llm = false;
    std::filesystem::path input_file;
    std::optional<std::filesystem::path> instructions_file;
    std::optional<std::filesystem::path> output_file;
    std::optional<std::string> instructions_from_cli;
    std::string model = "gpt-4o";

    void validate_params();
};

} // namespace params
