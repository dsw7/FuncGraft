#pragma once

#include <filesystem>
#include <optional>
#include <string>

struct Configurations {
    void load_configs_from_config_file();
    void validate_params();

    bool verbose = false;

    std::filesystem::path input_file;
    std::optional<std::filesystem::path> instructions_file;
    std::optional<std::filesystem::path> output_file;
    std::optional<std::string> instructions_from_cli;

    int port_ollama = 11434;
    std::string provider;
    std::string host_ollama;
    std::string model_ollama;
    std::string model_openai;
};
