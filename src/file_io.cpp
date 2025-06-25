#include "file_io.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <stdexcept>

namespace {

const std::string DELIMITER_LINE_ = "@@@@@\n";

struct Parts {
    std::string head;
    std::string core;
    std::string tail;
};

Parts unpack_text_into_parts(const std::string &text)
{
    const std::string::size_type idx_head = text.find(DELIMITER_LINE_);

    Parts parts;
    parts.head = text.substr(0, idx_head);

    const std::string::size_type idx_start_core = idx_head + 6;
    const std::string::size_type idx_tail = text.find(DELIMITER_LINE_, idx_start_core);

    if (idx_tail == std::string::npos) {
        throw std::runtime_error("No matching closing delimiter line");
    }

    const std::string::size_type size_core = idx_tail - idx_start_core;
    parts.core = text.substr(idx_start_core, size_core);
    parts.tail = text.substr(idx_tail + 6);
    return parts;
}

} // namespace

namespace file_io {

void FileIO::load_input_text_from_file(const std::filesystem::path &filename)
{
    if (not std::filesystem::exists(filename)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", filename.string()));
    }

    if (not std::filesystem::is_regular_file(filename)) {
        throw std::runtime_error(fmt::format("Input '{}' is not a file!", filename.string()));
    }

    const std::string file_contents = utils::read_from_file(filename);

    if (file_contents.contains(DELIMITER_LINE_)) {
        const Parts parts = unpack_text_into_parts(file_contents);
        this->head_ = parts.head;
        this->core_ = parts.core;
        this->tail_ = parts.tail;
        this->is_delimited_ = true;
    } else {
        this->text_ = file_contents;
        this->is_delimited_ = false;
    }
}

std::string FileIO::get_text()
{
    /*
     * If file contents follow:
     * -> "aaaa\n@@@@@\nbbbb\n@@@@@\ncccc\n"
     * Then return "bbbb\n" for processing
     *
     * If file contents follow:
     * -> "aaaa\nbbbb\ncccc\n"
     * Then return "aaaa\nbbbb\ncccc\n" for processing
     */

    if (this->is_delimited_) {
        if (not this->core_) {
            throw std::logic_error("The [core_] variable is unset!");
        }

        return this->core_.value();
    }

    if (not this->text_) {
        throw std::logic_error("The [text_] variable is unset!");
    }

    return this->text_.value();
}

void FileIO::set_text(const std::string &text)
{
    /*
     * If file contents were:
     * -> "aaaa\n@@@@@\nbbbb\n@@@@@\ncccc\n"
     * Then we overwrite only "bbbb\n" with whatever OpenAI returns
     *
     * If file contents follow:
     * -> "aaaa\nbbbb\ncccc\n"
     * Then we overwrite "aaaa\nbbbb\ncccc\n" with whatever OpenAI returns
     */

    if (this->is_delimited_) {
        this->core_ = text;
    } else {
        this->text_ = text;
    }
}

void FileIO::dump_output_text_to_file(const std::filesystem::path &filename)
{
    if (this->is_delimited_) {
        if (not this->head_) {
            throw std::logic_error("The [head_] variable is unset!");
        }
        if (not this->core_) {
            throw std::logic_error("The [core_] variable is unset!");
        }
        if (not this->tail_) {
            throw std::logic_error("The [tail_] variable is unset!");
        }

        utils::write_to_file(filename, this->head_.value() + this->core_.value() + this->tail_.value());
        return;
    }

    if (not this->text_) {
        throw std::logic_error("The [text_] variable is unset!");
    }

    utils::write_to_file(filename, this->text_.value());
}

} // namespace file_io
