#include "responses_ollama.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace queries {

OllamaError::OllamaError(const std::string &response, const int status_code) :
    ErrorResponse(response, status_code)
{
    if (not this->json_.contains("error")) {
        throw std::runtime_error("An error occurred but 'error' key not found in the response JSON");
    }

    this->errmsg = this->json_["error"];
}

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

} // namespace queries
