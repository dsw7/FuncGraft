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
    this->core_original_ = content.substr(pos_end_1, pos_start_2 - pos_end_1);
    this->tail_ = content.substr(pos_end_2);
}

std::string DelimitedContent::get_core_original()
{
    return this->core_original_;
}

void DelimitedContent::set_core_modified(const std::string &modified_core)
{
    this->core_modified_ = modified_core;

    if (not this->core_modified_.empty() and this->core_modified_.back() != '\n') {
        this->core_modified_ += '\n';
    }
}

std::string DelimitedContent::pack_parts_into_content()
{
#ifdef TESTING_ENABLED
    return fmt::format(
        "{}{}{}",
        this->head_,
        this->core_modified_,
        this->tail_);
#else
    return fmt::format(
        "{}{}{}{}{}{}{}",
        this->head_,
        "<<<<<<< Original code\n",
        this->core_original_,
        "=======\n",
        this->core_modified_,
        ">>>>>>> Modified code\n",
        this->tail_);
#endif
}

Content::Content(const std::filesystem::path &filename)
{
    if (not std::filesystem::exists(filename)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", filename.string()));
    }

    if (not std::filesystem::is_regular_file(filename)) {
        throw std::runtime_error(fmt::format("Input '{}' is not a file!", filename.string()));
    }

    const std::string content = utils::read_from_file(filename);

    if (content.find(DELIMITER_LINE_) != std::string::npos) {
        DelimitedContent delim_content;
        delim_content.unpack_content_into_parts(content);
        this->content_ = delim_content;
    } else {
        this->content_ = content;
    }
}

std::string Content::get_file_content()
{
    if (std::holds_alternative<std::string>(this->content_)) {
        return std::get<std::string>(this->content_);
    }

    return std::get<DelimitedContent>(this->content_).get_core_original();
}

void Content::set_file_content(const std::string &content)
{
    if (std::holds_alternative<std::string>(this->content_)) {
        std::get<std::string>(this->content_) = content;
    } else {
        std::get<DelimitedContent>(this->content_).set_core_modified(content);
    }
}

bool Content::is_delimited()
{
    return std::holds_alternative<DelimitedContent>(this->content_);
}

void Content::export_content_to_file(const std::filesystem::path &filename)
{
    if (std::holds_alternative<std::string>(this->content_)) {
        utils::write_to_file(filename, std::get<std::string>(this->content_));
    } else {
        const std::string content = std::get<DelimitedContent>(this->content_).pack_parts_into_content();
        utils::write_to_file(filename, content);
    }
}

} // namespace file_io
} // namespace core
