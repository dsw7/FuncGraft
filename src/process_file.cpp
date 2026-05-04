#include "process_file.hpp"

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "configs.hpp"
#include "console.hpp"
#include "file_io.hpp"
#include "instructions.hpp"
#include "prompt.hpp"
#include "reporting.hpp"
#include "text_manip.hpp"
#include "utils.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <expected>
#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <thread>

namespace {

using adapters::OllamaResponse;
using OllamaResults = std::expected<OllamaResponse, adapters::OllamaError>;

using adapters::OpenAIResponse;
using OpenAIResults = std::expected<OpenAIResponse, adapters::OpenAIError>;

std::string total_time_to_hhmmss_(const double total_time_s)
{
    int h = static_cast<int>(total_time_s) / 3600;
    int m = (static_cast<int>(total_time_s) % 3600) / 60;
    int s = static_cast<int>(total_time_s) % 60;

    std::string result;

    if (h > 0) {
        result += std::to_string(h) + "h";
        if (m > 0 || s > 0) {
            result += " ";
        }
    }

    if (m > 0) {
        result += std::to_string(m) + "m";
        if (s > 0) {
            result += " ";
        }
    }

    if (s > 0 || result.empty()) {
        result += std::to_string(s) + "s";
    }

    return fmt::format("Total time: {}", result);
}

// Threading ------------------------------------------------------------------------------------------------

std::atomic<bool> TIMER_ENABLED(false);

void time_api_call_()
{
    const std::chrono::duration delay = std::chrono::milliseconds(100);

    static std::array spinner = { "⠋ ", "⠙ ", "⠹ ", "⠸ ", "⠼ ", "⠴ ", "⠦ ", "⠧ ", "⠇ ", "⠏ " };
    const int num_frames = spinner.size();

    while (TIMER_ENABLED.load()) {
        for (int i = 0; i < num_frames; ++i) {
            std::cout << " \r" << spinner[i] << std::flush;
            std::this_thread::sleep_for(delay);
        }
    }

    std::cout << " \r" << std::flush;
}

OpenAIResults run_openai_query_with_threading_(const Configurations &configs, const std::string &prompt)
{
    TIMER_ENABLED.store(true);
    std::thread timer(time_api_call_);

    std::optional<OpenAIResults> results;
    bool query_failed = false;
    std::string errmsg;

    try {
        results = adapters::OpenAI(configs).query_messages_api(prompt);
    } catch (std::runtime_error &e) {
        errmsg = e.what();
        query_failed = true;
    }

    TIMER_ENABLED.store(false);
    timer.join();

    if (query_failed) {
        throw std::runtime_error(errmsg);
    }

    return results.value();
}

OllamaResults run_ollama_query_with_threading_(const Configurations &configs, const std::string &prompt)
{
    TIMER_ENABLED.store(true);
    std::thread timer(time_api_call_);

    std::optional<OllamaResults> results;
    bool query_failed = false;
    std::string errmsg;

    try {
        results = adapters::Ollama(configs).query_generate_api(prompt);
    } catch (std::runtime_error &e) {
        errmsg = e.what();
        query_failed = true;
    }

    TIMER_ENABLED.store(false);
    timer.join();

    if (query_failed) {
        throw std::runtime_error(errmsg);
    }

    return results.value();
}

// ----------------------------------------------------------------------------------------------------------

template<typename T>
void report_query_info_(const T &response)
{
    if (response.was_refused) {
        fmt::print(fg(fmt::color::black) | bg(fmt::color::orange_red),
            " Refused | Input tokens: {} | Output tokens: {} ", response.input_tokens, response.output_tokens);
    } else {
        fmt::print(fg(fmt::color::white) | bg(fmt::color::dark_golden_rod),
            " Success | Input tokens: {} | Output tokens: {} ", response.input_tokens, response.output_tokens);
    }

    fmt::print("\n\n");

    if (response.was_refused) {
        fmt::print(fg(fmt::terminal_color::bright_yellow), "{}\n", response.description);
    } else {
        fmt::print(fg(fmt::color::dim_gray), "{}\n", response.description);
    }

    console::print_right_align(total_time_to_hhmmss_(response.total_time));
}

std::expected<std::string, std::string> edit_delimited_text_openai_(const Configurations &configs, const std::string &input_text)
{
    core::text_manip::Parts text_parts = core::text_manip::unpack_text_into_parts(input_text);

    if (core::text_manip::is_text_empty(text_parts.original_text)) {
        throw std::runtime_error("The delimited block does not contain any code");
    }

    core::reporting::print_code_being_targeted(text_parts.original_text);

    const std::string instructions = core::instructions::load_instructions(configs);
    const std::string prompt = core::prompt::build_prompt(instructions, text_parts.original_text, configs.input_file.extension());

    if (configs.verbose) {
        core::reporting::print_prompt(prompt);
    }

    const OpenAIResults results = run_openai_query_with_threading_(configs, prompt);
    if (not results) {
        throw std::runtime_error(results.error().errmsg);
    }

    report_query_info_(*results);

    if (results->was_refused) {
        return std::unexpected(results->description);
    }

    text_parts.modified_text = results->output_text;
    return pack_parts_into_text(text_parts);
}

std::expected<std::string, std::string> edit_delimited_text_ollama_(const Configurations &configs, const std::string &input_text)
{
    core::text_manip::Parts text_parts = core::text_manip::unpack_text_into_parts(input_text);

    if (core::text_manip::is_text_empty(text_parts.original_text)) {
        throw std::runtime_error("The delimited block does not contain any code");
    }

    core::reporting::print_code_being_targeted(text_parts.original_text);

    const std::string instructions = core::instructions::load_instructions(configs);
    const std::string prompt = core::prompt::build_prompt(instructions, text_parts.original_text, configs.input_file.extension());

    if (configs.verbose) {
        core::reporting::print_prompt(prompt);
    }

    const OllamaResults results = run_ollama_query_with_threading_(configs, prompt);
    if (not results) {
        throw std::runtime_error(results.error().errmsg);
    }

    report_query_info_(*results);

    if (results->was_refused) {
        return std::unexpected(results->description);
    }

    text_parts.modified_text = results->output_text;
    return pack_parts_into_text(text_parts);
}

std::expected<std::string, std::string> edit_full_text_openai_(const Configurations &configs, const std::string &input_text)
{
    const std::string instructions = core::instructions::load_instructions(configs);
    const std::string prompt = core::prompt::build_prompt(instructions, input_text, configs.input_file.extension());

    if (configs.verbose) {
        core::reporting::print_prompt(prompt);
    }

    const OpenAIResults results = run_openai_query_with_threading_(configs, prompt);
    if (not results) {
        throw std::runtime_error(results.error().errmsg);
    }

    report_query_info_(*results);

    if (results->was_refused) {
        return std::unexpected(results->description);
    }

    return results->output_text;
}

std::expected<std::string, std::string> edit_full_text_ollama_(const Configurations &configs, const std::string &input_text)
{
    const std::string instructions = core::instructions::load_instructions(configs);
    const std::string prompt = core::prompt::build_prompt(instructions, input_text, configs.input_file.extension());

    if (configs.verbose) {
        core::reporting::print_prompt(prompt);
    }

    const OllamaResults results = run_ollama_query_with_threading_(configs, prompt);
    if (not results) {
        throw std::runtime_error(results.error().errmsg);
    }

    report_query_info_(*results);

    if (results->was_refused) {
        return std::unexpected(results->description);
    }
    return results->output_text;
}

} // namespace

void process_file(const Configurations &configs)
{
    const std::string input_text = core::file_io::import_file_to_edit(configs.input_file);

    if (core::text_manip::is_text_empty(input_text)) {
        throw std::runtime_error("The file does not contain any code");
    }

    bool text_delimited = core::text_manip::is_text_delimited(input_text);
    std::expected<std::string, std::string> updated_code_or_error;

    if (text_delimited and configs.provider == "openai") {
        updated_code_or_error = edit_delimited_text_openai_(configs, input_text);
    } else if (text_delimited and configs.provider == "ollama") {
        updated_code_or_error = edit_delimited_text_ollama_(configs, input_text);
    } else if (not text_delimited and configs.provider == "openai") {
        updated_code_or_error = edit_full_text_openai_(configs, input_text);
    } else {
        updated_code_or_error = edit_full_text_ollama_(configs, input_text);
    }

    if (not updated_code_or_error) {
        fmt::print("Query was rejected\n");
        return;
    }

    const std::string edited_text = updated_code_or_error.value();

    if (configs.output_file) {
        fmt::print("Exported updated content to file '{}'\n", configs.output_file.value().string());
        core::file_io::export_edited_file(edited_text, configs.output_file.value());
        return;
    }

#ifndef TESTING_ENABLED
    console::print_separator();
    core::file_io::export_edited_file_with_prompt(edited_text, configs.input_file);
    console::print_separator();
#endif
}
