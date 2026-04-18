#include "process_file.hpp"

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "configs.hpp"
#include "file_io.hpp"
#include "generate_prompt.hpp"
#include "instructions.hpp"
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

fmt::terminal_color blue = fmt::terminal_color::bright_blue;

// Threading ------------------------------------------------------------------------------------------------

std::atomic<bool> TIMER_ENABLED(false);

void time_api_call_()
{
    const std::chrono::duration delay = std::chrono::milliseconds(100);

    static std::array spinner = { "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏" };
    const int num_frames = spinner.size();

    while (TIMER_ENABLED.load()) {
        for (int i = 0; i < num_frames; ++i) {
            std::cout << "\r" << spinner[i] << std::flush;
            std::this_thread::sleep_for(delay);
        }
    }

    std::cout << " \r" << std::flush;
}

OpenAIResults run_openai_query_with_threading_(const std::string &prompt)
{
    TIMER_ENABLED.store(true);
    std::thread timer(time_api_call_);

    std::optional<OpenAIResults> results;
    bool query_failed = false;
    std::string errmsg;

    try {
        results = adapters::OpenAI(configs.model_openai).query_messages_api(prompt);
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

OllamaResults run_ollama_query_with_threading_(const std::string &prompt)
{
    TIMER_ENABLED.store(true);
    std::thread timer(time_api_call_);

    std::optional<OllamaResults> results;
    bool query_failed = false;
    std::string errmsg;

    try {
        results = adapters::Ollama(configs.model_ollama).query_generate_api(prompt);
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

void print_code_being_targeted_(const std::string &code)
{
    utils::print_separator();
    fmt::print(fmt::emphasis::bold, "Delimited code:\n");
    fmt::print(fg(blue), "{}", code);
}

void print_prompt_if_verbose_(const std::string &prompt)
{
    utils::print_separator();
    fmt::print(fmt::emphasis::bold, "Prompt:\n");
    fmt::print(fg(blue), "{}", prompt);
}

void report_query_info_(const OpenAIResponse &response)
{
    utils::print_separator();
    fmt::print(fmt::emphasis::bold, "Information:\n");
    fmt::print("Input tokens: {}\n", response.input_tokens);
    fmt::print("Output tokens: {}\n", response.output_tokens);
    fmt::print("Description of changes: ");
    fmt::print(fg(blue), "{}\n", response.description);
}

void report_query_info_(const OllamaResponse &response)
{
    utils::print_separator();
    fmt::print(fmt::emphasis::bold, "Information:\n");
    fmt::print("Input tokens: {}\n", response.input_tokens);
    fmt::print("Output tokens: {}\n", response.output_tokens);
    fmt::print("Description of changes: ");
    fmt::print(fg(blue), "{}\n", response.description);
}

std::string edit_delimited_text_openai_(const params::CommandLineParameters &params, const std::string &input_text)
{
    pipeline::Parts text_parts = pipeline::unpack_text_into_parts(input_text);

    if (pipeline::is_text_empty(text_parts.original_text)) {
        throw std::runtime_error("The delimited block does not contain any code");
    }

    print_code_being_targeted_(text_parts.original_text);

    const std::string instructions = prompt::load_instructions(params);
    const std::string prompt = prompt::build_openai_prompt(instructions, text_parts.original_text, params.input_file.extension());

    if (params.verbose) {
        print_prompt_if_verbose_(prompt);
    }

    const OpenAIResults results = run_openai_query_with_threading_(prompt);
    if (not results) {
        throw std::runtime_error(results.error().errmsg);
    }

    report_query_info_(*results);

    text_parts.modified_text = results->output_text;
    return pack_parts_into_text(text_parts);
}

std::string edit_delimited_text_ollama_(const params::CommandLineParameters &params, const std::string &input_text)
{
    pipeline::Parts text_parts = pipeline::unpack_text_into_parts(input_text);

    if (pipeline::is_text_empty(text_parts.original_text)) {
        throw std::runtime_error("The delimited block does not contain any code");
    }

    print_code_being_targeted_(text_parts.original_text);

    const std::string instructions = prompt::load_instructions(params);
    const std::string prompt = prompt::build_ollama_prompt(instructions, text_parts.original_text, params.input_file.extension());

    if (params.verbose) {
        print_prompt_if_verbose_(prompt);
    }

    const OllamaResults results = run_ollama_query_with_threading_(prompt);
    if (not results) {
        throw std::runtime_error(results.error().errmsg);
    }

    report_query_info_(*results);

    text_parts.modified_text = results->output_text;
    return pack_parts_into_text(text_parts);
}

std::string edit_full_text_openai_(const params::CommandLineParameters &params, const std::string &input_text)
{
    const std::string instructions = prompt::load_instructions(params);
    const std::string prompt = prompt::build_openai_prompt(instructions, input_text, params.input_file.extension());

    if (params.verbose) {
        print_prompt_if_verbose_(prompt);
    }

    const OpenAIResults results = run_openai_query_with_threading_(prompt);
    if (not results) {
        throw std::runtime_error(results.error().errmsg);
    }

    report_query_info_(*results);

    return results->output_text;
}

std::string edit_full_text_ollama_(const params::CommandLineParameters &params, const std::string &input_text)
{
    const std::string instructions = prompt::load_instructions(params);
    const std::string prompt = prompt::build_ollama_prompt(instructions, input_text, params.input_file.extension());

    if (params.verbose) {
        print_prompt_if_verbose_(prompt);
    }

    const OllamaResults results = run_ollama_query_with_threading_(prompt);
    if (not results) {
        throw std::runtime_error(results.error().errmsg);
    }

    report_query_info_(*results);

    return results->output_text;
}

} // namespace

namespace pipeline {

void process_file(const params::CommandLineParameters &params)
{
    const std::string input_text = import_file_to_edit(params.input_file);

    if (is_text_empty(input_text)) {
        throw std::runtime_error("The file does not contain any code");
    }

    bool text_delimited = is_text_delimited(input_text);
    std::string output_text;

    if (text_delimited and configs.provider == "openai") {
        output_text = edit_delimited_text_openai_(params, input_text);
    } else if (text_delimited and configs.provider == "ollama") {
        output_text = edit_delimited_text_ollama_(params, input_text);
    } else if (not text_delimited and configs.provider == "openai") {
        output_text = edit_full_text_openai_(params, input_text);
    } else {
        output_text = edit_full_text_ollama_(params, input_text);
    }

    if (params.output_file) {
        fmt::print("Exported updated content to file '{}'\n", params.output_file.value().string());
        export_edited_file(output_text, params.output_file.value());
        return;
    }

#ifndef TESTING_ENABLED
    utils::print_separator();
    export_edited_file_with_prompt(output_text, params.input_file);
    utils::print_separator();
#endif
}

} // namespace pipeline
