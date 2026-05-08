#include "code.hpp"

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

std::string DelimitedCode::get_core_original() const
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

CodeToEdit::CodeToEdit(const std::string &original_code)
{
    if (original_code.find(DELIMITER_LINE_) != std::string::npos) {
        DelimitedCode delimited_code;
        delimited_code.unpack_content_into_parts(original_code);
        this->code_ = delimited_code;
    } else {
        this->code_ = original_code;
    }
}

std::string CodeToEdit::get_original_code() const
{
    if (std::holds_alternative<std::string>(this->code_)) {
        return std::get<std::string>(this->code_);
    }

    return std::get<DelimitedCode>(this->code_).get_core_original();
}

void CodeToEdit::overwrite_original_code(const std::string &modified_code)
{
    if (std::holds_alternative<std::string>(this->code_)) {
        std::get<std::string>(this->code_) = modified_code;
    } else {
        std::get<DelimitedCode>(this->code_).set_core_modified(modified_code);
    }
}

bool CodeToEdit::is_delimited()
{
    return std::holds_alternative<DelimitedCode>(this->code_);
}

std::string CodeToEdit::get_modified_code()
{
    if (std::holds_alternative<std::string>(this->code_)) {
        return std::get<std::string>(this->code_);
    }

    return std::get<DelimitedCode>(this->code_).pack_parts_into_content();
}

} // namespace code
} // namespace core
