#include "system_prompts.hpp"

namespace system_prompts {

std::string system_prompt_edit_code()
{
    return R"(
You are a helpful assistant that specializes in programming.
The user will provide some code and instructions on what to do with the code.

IMPORTANT: Do not wrap your response in backticks (```). Output the code
directly without markdown code fences.

Output:
- description_of_changes: brief summary of the changes you applied
- code: your updated code
)";
}

} // namespace system_prompts
