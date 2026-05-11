#include "responses.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace adapters {

SuccessResponse::SuccessResponse(const std::string &response)
{
    try {
        this->response_ = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse response: {}", e.what()));
    }
}

} // namespace adapters
