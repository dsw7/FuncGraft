#pragma once

#include <filesystem>
#include <optional>
#include <string>

struct Configurations {
    void validate_configs_from_cli();

    bool verbose = false;
    std::filesystem::path input_file;
    std::optional<std::filesystem::path> instructions_file;
    std::optional<std::filesystem::path> output_file;
    std::optional<std::string> instructions_from_cli;

    void validate_configs_from_file();

    int port_ollama = 11434;
    std::string host_ollama;
    std::string model_ollama;
    std::string model_openai;

    std::optional<std::string> provider;
};
