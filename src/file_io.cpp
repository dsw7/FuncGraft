#include "file_io.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace {

const std::string DELIMITER_LINE_ = "@@@@@\n";

}

namespace file_io {

void FileIO::load_input_text(const std::filesystem::path &filename)
{
    if (not std::filesystem::exists(filename)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", filename.string()));
    }

    if (not std::filesystem::is_regular_file(filename)) {
        throw std::runtime_error(fmt::format("Input '{}' is not a file!", filename.string()));
    }

    const std::string file_contents = utils::read_from_file(filename);

    if (file_contents.contains(DELIMITER_LINE_)) {
        this->is_delimited_ = true;
    } else {
        this->is_delimited_ = false;
        this->text_ = file_contents;
    }
}

void FileIO::dump_output_text(const std::filesystem::path &filename)
{
    if (not this->is_delimited_) {
        if (this->text_) {
            utils::write_to_file(filename, this->text_.value());
        } else {
            throw std::logic_error("The [text_] variable is unset!");
        }

        return;
    }
}

} // namespace file_io
