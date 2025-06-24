#include "utils.hpp"

#include <fmt/core.h>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {

unsigned short get_terminal_columns()
{
    static struct winsize window_size;
    window_size.ws_col = 0;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == 0) {
        return window_size.ws_col;
    }

    return 20;
}

} // namespace

namespace utils {

void print_separator()
{
    static unsigned short columns = get_terminal_columns();
    static std::string separator = std::string(columns, '-');

    fmt::print("{}\n", separator);
}

std::string read_from_file(const std::string &filename)
{
    std::ifstream file(filename);

    if (not file.is_open()) {
        throw std::runtime_error(fmt::format("Unable to open '{}' for reading", filename));
    }

    std::stringstream buffer;

    buffer << file.rdbuf();
    file.close();

    const std::string text = buffer.str();
    return text;
}

void write_to_file(const std::string &filename, const std::string &text)
{
    std::ofstream file(filename);

    if (not file.is_open()) {
        throw std::runtime_error(fmt::format("Unable to open '{}' for writing", filename));
    }

    file << text;
    file.close();
}

std::optional<std::string> resolve_label_from_extension(const std::string &extension)
{
    static std::map<std::string, std::string> ext_to_label {
        { ".bash", "bash" },
        { ".sh", "sh" },
        { ".c", "c" },
        { ".cpp", "cpp" },
        { ".c++", "cpp" },
        { ".cs", "csharp" },
        { ".csharp", "csharp" },
        { ".css", "css" },
        { ".html", "html" },
        { ".javascript", "javascript" },
        { ".js", "javascript" },
        { ".json", "json" },
        { ".java", "java" },
        { ".kotlin", "kotlin" },
        { ".perl", "perl" },
        { ".php", "php" },
        { ".python", "python" },
        { ".py", "python" },
        { ".ruby", "ruby" },
        { ".rust", "rust" },
        { ".sql", "sql" },
        { ".swift", "swift" },
        { ".typescript", "typescript" },
        { ".ts", "typescript" },
        { ".xml", "xml" },
        { ".yaml", "yaml" },
        { ".yml", "yaml" },
    };

    if (ext_to_label.contains(extension)) {
        return ext_to_label[extension];
    }

    return std::nullopt;
}

std::string get_code_block(const std::string &body)
{
    if (body.back() == '\n') {
        return fmt::format("```\n{}```\n", body);
    }

    return fmt::format("```\n{}\n```\n", body);
}

std::string get_code_block(const std::string &body, const std::string &label)
{
    if (body.back() == '\n') {
        return fmt::format("```{}\n{}```\n", label, body);
    }

    return fmt::format("```{}\n{}\n```\n", label, body);
}

} // namespace utils
