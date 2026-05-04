#include "console.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <sys/ioctl.h>

namespace {

unsigned short get_terminal_columns_()
{
    static struct winsize window_size;
    window_size.ws_col = 0;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == 0) {
        return window_size.ws_col;
    }

    return 20;
}

} // namespace

namespace console {

void print_separator()
{
    static unsigned short columns = get_terminal_columns_();
    static std::string separator = std::string(columns, '-');

    fmt::print(fg(fmt::color::gray), "{}\n", separator);
}

} // namespace console
