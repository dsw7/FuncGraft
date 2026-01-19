#include "params.hpp"

#include <stdexcept>

namespace params {

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

} // namespace params
