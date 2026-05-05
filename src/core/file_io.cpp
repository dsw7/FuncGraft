#include "file_io.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <stdexcept>
#include <vector>

namespace {
const std::string DELIMITER_LINE_ = "@@@\n";
}

namespace core {
namespace file_io {

void DelimitedContent::unpack_content_into_parts(const std::string &content)
{
    /*
     * Unpack text of form:
     * -> "aaaa\n@@@\nbbbb\n@@@\ncccc\n"
     * Then return "bbbb\n" for processing
     *
     * To:
     * head: aaaa\n
     * core: bbbb\n
     * tail: cccc\n
     *
     * So we can operate on the "core" only
     */
    static std::size_t size_delim_line = DELIMITER_LINE_.size();

    std::vector<std::size_t> indices;
    std::size_t pos = content.find(DELIMITER_LINE_);

    while (pos != std::string::npos) {
        indices.push_back(pos);
        pos = content.find(DELIMITER_LINE_, pos + size_delim_line);
    }

    const int num_indices = indices.size();

    if (num_indices == 1) {
        throw std::runtime_error("No matching closing delimiter line");
    } else if (num_indices != 2) {
        throw std::runtime_error("The number of delimiter lines must be exactly 2");
    }

    std::size_t pos_start_1 = indices[0];
    std::size_t pos_end_1 = indices[0] + size_delim_line;
    std::size_t pos_start_2 = indices[1];
    std::size_t pos_end_2 = indices[1] + size_delim_line;

    this->head_ = content.substr(0, pos_start_1);
    this->core_ = content.substr(pos_end_1, pos_start_2 - pos_end_1);
    this->tail_ = content.substr(pos_end_2);
}

std::string DelimitedContent::get_core()
{
    return this->core_;
}

void DelimitedContent::set_modified_core(const std::string &modified_core)
{
    this->modified_core_ = modified_core;

    if (not this->modified_core_.empty() and this->modified_core_.back() != '\n') {
        this->modified_core_ += '\n';
    }
}

std::string DelimitedContent::pack_parts_into_content()
{
#ifdef TESTING_ENABLED
    return fmt::format(
        "{}{}{}",
        this->head_,
        this->modified_core_,
        this->tail_);
#else
    static std::string marker_original = "<<<<<<< Original code\n";
    static std::string marker_split = "=======\n";
    static std::string marker_modified = ">>>>>>> Modified code\n";

    return fmt::format(
        "{}{}{}{}{}{}{}",
        this->head_,
        marker_original,
        this->core_,
        marker_split,
        this->modified_core_,
        marker_modified,
        this->tail_);
#endif
}

FileToEdit::FileToEdit(const std::filesystem::path &filename)
{
    if (not std::filesystem::exists(filename)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", filename.string()));
    }

    if (not std::filesystem::is_regular_file(filename)) {
        throw std::runtime_error(fmt::format("Input '{}' is not a file!", filename.string()));
    }

    const std::string content = utils::read_from_file(filename);
    this->is_delimited_ = content.find(DELIMITER_LINE_) != std::string::npos;

    if (this->is_delimited_) {
        this->delim_content_.unpack_content_into_parts(content);
    } else {
        this->content_ = content;
    }
}

std::string FileToEdit::get_file_content()
{
    if (this->is_delimited_) {
        return this->delim_content_.get_core();
    }

    return this->content_;
}

void FileToEdit::set_file_content(const std::string &content)
{
    if (this->is_delimited_) {
        this->delim_content_.set_modified_core(content);
    } else {
        this->content_ = content;
    }
}

void FileToEdit::export_content(const std::filesystem::path &filename)
{
    if (this->is_delimited_) {
        utils::write_to_file(filename, this->delim_content_.pack_parts_into_content());
    } else {
        utils::write_to_file(filename, this->content_);
    }
}

} // namespace file_io
} // namespace core
