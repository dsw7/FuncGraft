#include "code.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <stdexcept>
#include <vector>

namespace {
const std::string DELIMITER_LINE_ = "@@@\n";
}

namespace core {
namespace code {

void DelimitedCode::unpack_content_into_parts(const std::string &content)
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

std::string DelimitedCode::get_core_original()
{
    return this->core_original_;
}

void DelimitedCode::set_core_modified(const std::string &modified_core)
{
    this->core_modified_ = modified_core;

    if (not this->core_modified_.empty() and this->core_modified_.back() != '\n') {
        this->core_modified_ += '\n';
    }
}

std::string DelimitedCode::pack_parts_into_content()
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

CodeToEdit::CodeToEdit(const std::string &raw_text)
{
    if (raw_text.find(DELIMITER_LINE_) != std::string::npos) {
        DelimitedCode delim_content;
        delim_content.unpack_content_into_parts(raw_text);
        this->content_ = delim_content;
    } else {
        this->content_ = raw_text;
    }
}

std::string CodeToEdit::get_file_content()
{
    if (std::holds_alternative<std::string>(this->content_)) {
        return std::get<std::string>(this->content_);
    }

    return std::get<DelimitedCode>(this->content_).get_core_original();
}

void CodeToEdit::set_file_content(const std::string &content)
{
    if (std::holds_alternative<std::string>(this->content_)) {
        std::get<std::string>(this->content_) = content;
    } else {
        std::get<DelimitedCode>(this->content_).set_core_modified(content);
    }
}

bool CodeToEdit::is_delimited()
{
    return std::holds_alternative<DelimitedCode>(this->content_);
}

void CodeToEdit::export_content_to_file(const std::filesystem::path &filename)
{
    if (std::holds_alternative<std::string>(this->content_)) {
        utils::write_to_file(filename, std::get<std::string>(this->content_));
    } else {
        const std::string content = std::get<DelimitedCode>(this->content_).pack_parts_into_content();
        utils::write_to_file(filename, content);
    }
}

} // namespace code
} // namespace core
