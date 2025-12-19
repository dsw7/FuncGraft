#pragma once

#include <string>

namespace prompt {
std::string build_openai_prompt(const std::string &instructions, const std::string &input_text, const std::string &extension);
std::string build_ollama_prompt(const std::string &instructions, const std::string &input_text, const std::string &extension);
} // namespace prompt
