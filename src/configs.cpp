#include "configs.hpp"

#include <array>
#include <stdexcept>

void Configurations::validate_configs_from_cli()
{

    if (this->instructions_from_cli) {
        if (this->instructions_from_cli.value().empty()) {
            throw std::invalid_argument("CLI instructions are empty. Cannot proceed");
        }
    }
}

void Configurations::validate_input_file_()
{
    if (this->input_file.empty()) {
        throw std::invalid_argument("No filename was provided. Cannot proceed");
    }
}

void Configurations::validate_instructions_file_()
{
    if (this->instructions_file) {
        if (this->instructions_file.value().empty()) {
            throw std::invalid_argument("Instructions filename was not provided. Cannot proceed");
        }
    }
}

void Configurations::validate_output_file_()
{
    if (this->output_file) {
        if (this->output_file.value().empty()) {
            throw std::invalid_argument("Output filename was not provided. Cannot proceed");
        }
    }
}

void Configurations::validate_provider_()
{
    if (not this->provider) {
        throw std::runtime_error("LLM provider is unset");
    }

    const std::array<std::string, 2> providers = { "ollama", "openai" };

    for (const auto &it: providers) {
        if (it == this->provider) {
            return;
        }
    }

    throw std::runtime_error("Invalid LLM provider");
}

void Configurations::validate_configurations()
{
    this->validate_input_file_();
    this->validate_instructions_file_();
    this->validate_output_file_();
    this->validate_provider_();
}
