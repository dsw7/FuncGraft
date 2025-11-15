#include "file_io.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <stdexcept>
#include <vector>

namespace {

const std::string DELIMITER_LINE_ = "@@@\n";
const std::size_t SIZE_DELIM_LINE_ = DELIMITER_LINE_.size();

bool is_text_delimited(const std::string &text)
{
    return text.find(DELIMITER_LINE_) != std::string::npos;
}

struct Positions {
    std::size_t pos_start_1 = 0;
    std::size_t pos_end_1 = 0;
    std::size_t pos_start_2 = 0;
    std::size_t pos_end_2 = 0;
};

Positions get_delimiter_positions(const std::string &text)
{
    std::vector<std::size_t> indices;
    std::size_t pos = text.find(DELIMITER_LINE_);

    while (pos != std::string::npos) {
        indices.push_back(pos);
        pos = text.find(DELIMITER_LINE_, pos + SIZE_DELIM_LINE_);
    }

    int num_indices = indices.size();

    if (num_indices == 1) {
        throw std::runtime_error("No matching closing delimiter line");
    } else if (num_indices != 2) {
        throw std::runtime_error("The number of delimiter lines must be exactly 2");
    }

    Positions positions;
    positions.pos_start_1 = indices[0];
    positions.pos_end_1 = indices[0] + SIZE_DELIM_LINE_;
    positions.pos_start_2 = indices[1];
    positions.pos_end_2 = indices[1] + SIZE_DELIM_LINE_;
    return positions;
}

struct Parts {
    std::string head;
    std::string core;
    std::string tail;
};

Parts unpack_text_into_parts(const std::string &text)
{
    const Positions positions = get_delimiter_positions(text);

    Parts parts;
    parts.head = text.substr(0, positions.pos_start_1);
    parts.core = text.substr(positions.pos_end_1, positions.pos_start_2 - positions.pos_end_1);
    parts.tail = text.substr(positions.pos_end_2);
    return parts;
}

std::string pack_parts_into_text(const Parts &parts)
{
    return parts.head + parts.core + parts.tail;
}

} // namespace

namespace file_io {

void FileIO::load_input_text_from_file(const std::filesystem::path &filename)
{
    const std::string file_contents = utils::read_from_file(filename);

    if (is_text_delimited(file_contents)) {
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
     * -> "aaaa\n@@@\nbbbb\n@@@\ncccc\n"
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
     * -> "aaaa\n@@@\nbbbb\n@@@\ncccc\n"
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

std::string FileIO::dump_output_text_to_string()
{
    std::string output;

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
        Parts parts;
        parts.head = this->head_.value();
        parts.core = this->core_.value();
        parts.tail = this->tail_.value();
        output = pack_parts_into_text(parts);
    } else {
        if (not this->text_) {
            throw std::logic_error("The [text_] variable is unset!");
        }

        output = this->text_.value();
    }

    return output;
}

void FileIO::dump_output_text_to_file(const std::filesystem::path &filename)
{
    utils::write_to_file(filename, this->dump_output_text_to_string());
}

} // namespace file_io
