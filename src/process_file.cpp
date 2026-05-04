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

// Threading ------------------------------------------------------------------------------------------------

std::atomic<bool> TIMER_ENABLED_(false);

void time_api_call_()
{
    const std::chrono::duration delay = std::chrono::milliseconds(100);

    static std::array spinner = { "⠋ ", "⠙ ", "⠹ ", "⠸ ", "⠼ ", "⠴ ", "⠦ ", "⠧ ", "⠇ ", "⠏ " };
    const int num_frames = spinner.size();

    while (TIMER_ENABLED_.load()) {
        for (int i = 0; i < num_frames; ++i) {
            std::cout << " \r" << spinner[i] << std::flush;
            std::this_thread::sleep_for(delay);
        }
    }

    std::cout << " \r" << std::flush;
}

OpenAIResults run_openai_query_with_threading_(const Configurations &configs, const std::string &prompt)
{
    TIMER_ENABLED_.store(true);
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

    TIMER_ENABLED_.store(false);
    timer.join();

    if (query_failed) {
        throw std::runtime_error(errmsg);
    }

    return results.value();
}

OllamaResults run_ollama_query_with_threading_(const Configurations &configs, const std::string &prompt)
{
    TIMER_ENABLED_.store(true);
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

    TIMER_ENABLED_.store(false);
    timer.join();

    if (query_failed) {
        throw std::runtime_error(errmsg);
    }

    return results.value();
}

// ----------------------------------------------------------------------------------------------------------

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

    core::reporting::print_query_info(*results);

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

    core::reporting::print_query_info(*results);

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

    core::reporting::print_query_info(*results);

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

    core::reporting::print_query_info(*results);

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
