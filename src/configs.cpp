#include "configs.hpp"

#include "toml.hpp"

#include <filesystem>
#include <fmt/core.h>
#include <stdexcept>
#include <stdlib.h>

namespace {

namespace fs = std::filesystem;

fs::path get_proj_data_dir()
{
    const char *home_dir = std::getenv("HOME");

    if (not home_dir) {
        throw std::runtime_error("Could not locate user home directory!");
    }

    const std::string proj_dir = std::string(home_dir) + "/.funcgraft";

    if (not fs::exists(proj_dir)) {
        throw std::runtime_error(fmt::format("Could not locate '{}'", proj_dir));
    }

    return fs::path(proj_dir);
}

} // namespace

void CommandLineParameters::validate_params()
{
    if (this->input_file.empty()) {
        throw std::invalid_argument("No filename was provided. Cannot proceed");
    }

    if (this->instructions_file) {
        if (this->instructions_file.value().empty()) {
            throw std::invalid_argument("Instructions filename was not provided. Cannot proceed");
        }
    }

    if (this->output_file) {
        if (this->output_file.value().empty()) {
            throw std::invalid_argument("Output filename was not provided. Cannot proceed");
        }
    }

    if (this->instructions_from_cli) {
        if (this->instructions_from_cli.value().empty()) {
            throw std::invalid_argument("CLI instructions are empty. Cannot proceed");
        }
    }
}

void CommandLineParameters::load_configs_from_config_file()
{
    static fs::path proj_config = get_proj_data_dir() / "funcgraft.toml";

    if (not fs::exists(proj_config)) {
        throw std::runtime_error("Could not locate FuncGraft configuration file!");
    }

    toml::table table;

    try {
        table = toml::parse_file(proj_config.string());
    } catch (const toml::parse_error &e) {
        throw std::runtime_error(e);
    }

    this->provider = table["general"]["provider"].value_or("openai");
    this->host_ollama = table["ollama"]["host"].value_or("localhost");
    this->model_ollama = table["ollama"]["model"].value_or("gemma3:latest");
    this->model_openai = table["openai"]["model"].value_or("gpt-4o");
    this->port_ollama = table["ollama"]["port"].value_or(11434);

    if (this->provider != "ollama" and this->provider != "openai") {
        throw std::runtime_error("Provider must be one of \"openai\" or \"ollama\"");
    }
}
