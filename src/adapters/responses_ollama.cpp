#include "responses_ollama.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace adapters {

OllamaResponse::OllamaResponse(const std::string &response) :
    SuccessResponse(response)
{
    if (not this->response_.contains("done")) {
        throw std::runtime_error("The response from Ollama does not contain the 'done' key");
    }

    if (not this->response_["done"]) {
        throw std::runtime_error("The response from Ollama indicates the job is not done");
    }
}

} // namespace adapters
