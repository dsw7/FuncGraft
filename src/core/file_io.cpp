#include "file_io.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <stdexcept>
#include <vector>

namespace {
const std::string DELIMITER_LINE_ = "@@@\n";

class DelimitedContent_ {
public:
    void unpack_text_into_parts(const std::string &content);
    std::string get_core();
    std::string pack_parts_into_text(std::string &modified_core);

private:
    std::string head_;
    std::string core_;
    std::string tail_;
};

void DelimitedContent_::unpack_text_into_parts(const std::string &content)
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

std::string DelimitedContent_::get_core()
{
    return this->core_;
}

std::string DelimitedContent_::pack_parts_into_text(std::string &modified_core)
{
    if (not modified_core.empty() and modified_core.back() != '\n') {
        modified_core += '\n';
    }

#ifdef TESTING_ENABLED
    return fmt::format(
        "{}{}{}",
        this->head_,
        modified_core,
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
        modified_core,
        marker_modified,
        this->tail_);
#endif
}

} // namespace

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
    this->is_delimited_ = this->content_.find(DELIMITER_LINE_) != std::string::npos;
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
