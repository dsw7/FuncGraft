#include "file_io.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace core {
namespace file_io {

FileToEdit::FileToEdit(const std::filesystem::path &filename)
{
    if (not std::filesystem::exists(filename)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", filename.string()));
    }

    if (not std::filesystem::is_regular_file(filename)) {
        throw std::runtime_error(fmt::format("Input '{}' is not a file!", filename.string()));
    }

    this->content_ = utils::read_from_file(filename);
}

std::string FileToEdit::get_file_content()
{
    return this->content_;
}

void FileToEdit::set_file_content(const std::string &content)
{
    this->content_ = content;
}

void FileToEdit::export_content(const std::filesystem::path &filename)
{
    utils::write_to_file(filename, this->content_);
}

} // namespace file_io
} // namespace core
