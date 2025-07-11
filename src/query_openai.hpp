#pragma once

#include <string>

namespace query_openai {

struct QueryResults {
    int completion_tokens = 0;
    int prompt_tokens = 0;
    std::string description;
    std::string output_text;
};

QueryResults run_query(const std::string &prompt, const std::string &model);

} // namespace query_openai
