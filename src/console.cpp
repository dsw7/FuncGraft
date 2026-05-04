#include "console.hpp"

#include "utils.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <sys/ioctl.h>

namespace console {

void print_right_align(const std::string &message)
{
    static unsigned short width = utils::get_term_width();
    static unsigned short r_pad = 2;

    const unsigned short l_pad = width - r_pad;
    fmt::print(fg(fmt::color::gray), "{:>{}}\n", message, l_pad);
}

} // namespace console
