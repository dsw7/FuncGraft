#include "params.hpp"

#include <stdexcept>

namespace params {

void CommandLineParameters::validate_params()
{
    if (this->input_file.empty()) {
        throw std::invalid_argument("No filename was provided. Cannot proceed");
    }

    if (this->model.empty()) {
        throw std::invalid_argument("Model is empty. Cannot proceed");
    }
}

} // namespace params
