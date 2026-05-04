#include "console.hpp"

#include "utils.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <sys/ioctl.h>

namespace {

std::string build_separator_()
{
    static unsigned short width = utils::get_term_width();
    std::string s;

    for (short i = 0; i < width; i++) {
        s += "\u2500";
    }

    return s;
}

} // namespace

namespace console {

void print_separator()
{
    static std::string separator = build_separator_();
    fmt::print(fg(fmt::color::gray), "{}\n", separator);
}

void print_right_align(const std::string &message)
{
    static unsigned short width = utils::get_term_width();
    static unsigned short r_pad = 2;

    const unsigned short l_pad = width - r_pad;
    fmt::print(fg(fmt::color::gray), "{:>{}}\n", message, l_pad);
}

} // namespace console
