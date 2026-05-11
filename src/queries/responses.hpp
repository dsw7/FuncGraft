#pragma once

#include <json.hpp>
#include <string>

namespace adapters {

class ErrorResponse {
public:
    ErrorResponse(const std::string &response, const int status_code);
    std::string errmsg;
    int status_code = 200;

protected:
    nlohmann::json json_;
};

class SuccessResponse {
public:
    SuccessResponse() = default;
    SuccessResponse(const std::string &response);

protected:
    nlohmann::json response_;
};

} // namespace adapters
