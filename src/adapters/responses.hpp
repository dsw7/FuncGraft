#pragma once

#include <json.hpp>
#include <string>

namespace adapters {

class SuccessResponse {
public:
    SuccessResponse() = default;
    SuccessResponse(const std::string &response);

protected:
    nlohmann::json response_;
};

} // namespace adapters
