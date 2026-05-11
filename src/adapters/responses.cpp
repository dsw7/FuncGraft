#include "responses.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace adapters {

ErrorResponse::ErrorResponse(const std::string &response, const int status_code) :
    status_code(status_code)
{
    try {
        this->json_ = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse error response: {}", e.what()));
    }
}

SuccessResponse::SuccessResponse(const std::string &response)
{
    try {
        this->response_ = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse success response: {}", e.what()));
    }
}

} // namespace adapters
