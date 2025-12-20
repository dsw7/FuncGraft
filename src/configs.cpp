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

void Configs::load_configs_from_config_file()
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

    this->host_ollama = table["ollama"]["host"].value_or("localhost");
    this->model_ollama = table["ollama"]["model"].value_or("gemma3:latest");
    this->model_openai = table["openai"]["model"].value_or("gpt-4o");
    this->port_ollama = table["ollama"]["port"].value_or(11434);
}

Configs configs;
