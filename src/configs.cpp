#include "configs.hpp"

#include <stdexcept>

void Configurations::validate_configs_from_cli()
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

void Configurations::validate_configs_from_file()
{
    if (this->provider != "ollama" and this->provider != "openai") {
        throw std::runtime_error("Provider must be one of \"openai\" or \"ollama\"");
    }
}
