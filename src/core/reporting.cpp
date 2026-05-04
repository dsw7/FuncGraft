#include "reporting.hpp"

#include "console.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

namespace {

std::string total_time_to_hhmmss_(const double total_time_s)
{
    int h = static_cast<int>(total_time_s) / 3600;
    int m = (static_cast<int>(total_time_s) % 3600) / 60;
    int s = static_cast<int>(total_time_s) % 60;

    std::string result;

    if (h > 0) {
        result += std::to_string(h) + "h";
        if (m > 0 || s > 0) {
            result += " ";
        }
    }

    if (m > 0) {
        result += std::to_string(m) + "m";
        if (s > 0) {
            result += " ";
        }
    }

    if (s > 0 || result.empty()) {
        result += std::to_string(s) + "s";
    }

    return fmt::format("Total time: {}", result);
}

template<typename T>
void print_query_info_(const T &response)
{
    if (response.was_refused) {
        fmt::print(fg(fmt::color::black) | bg(fmt::color::orange_red),
            " Refused | Input tokens: {} | Output tokens: {} ", response.input_tokens, response.output_tokens);
    } else {
        fmt::print(fg(fmt::color::white) | bg(fmt::color::dark_golden_rod),
            " Success | Input tokens: {} | Output tokens: {} ", response.input_tokens, response.output_tokens);
    }

    fmt::print("\n\n");

    if (response.was_refused) {
        fmt::print(fg(fmt::terminal_color::bright_yellow), "{}\n", response.description);
    } else {
        fmt::print(fg(fmt::color::dim_gray), "{}\n", response.description);
    }

    console::print_right_align(total_time_to_hhmmss_(response.total_time));
}

} // namespace

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

void print_query_info(const adapters::OllamaResponse &response)
{
    print_query_info_(response);
}

void print_query_info(const adapters::OpenAIResponse &response)
{
    print_query_info_(response);
}

} // namespace reporting
} // namespace core
