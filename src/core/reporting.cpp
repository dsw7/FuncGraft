#include "reporting.hpp"

#include "console.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

namespace core {
namespace reporting {

void print_code_being_targeted(const std::string &code)
{
    console::print_separator();
    fmt::print(fg(fmt::color::dim_gray), "@@@\n");
    fmt::print(fg(fmt::terminal_color::bright_blue), "{}", code);
    fmt::print(fg(fmt::color::dim_gray), "@@@\n");
}

void print_prompt(const std::string &prompt)
{
    fmt::print(fmt::emphasis::bold, "Prompt:\n");
    fmt::print(fg(fmt::terminal_color::bright_blue), "{}", prompt);
    console::print_separator();
}

} // namespace reporting
} // namespace core
