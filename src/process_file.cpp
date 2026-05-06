#include "process_file.hpp"

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "configs.hpp"
#include "file_io.hpp"
#include "prompt.hpp"
#include "reporting.hpp"
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

bool is_text_empty_(const std::string &input_text)
{
    if (input_text.empty()) {
        return true;
    }

    return std::all_of(input_text.begin(), input_text.end(), [](char c) {
        return std::isspace(static_cast<unsigned char>(c));
    });
}

std::expected<std::string, std::string> edit_full_text_openai_(const Configurations &configs, const std::string &input_text)
{
    const std::string prompt = core::prompt::build_prompt(configs, input_text);

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
    const std::string prompt = core::prompt::build_prompt(configs, input_text);

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
    core::file_io::Content content(configs.input_file);
    std::string input_text = content.get_file_content();

    if (is_text_empty_(input_text)) {
        throw std::runtime_error("No code to edit");
    }

    if (content.is_delimited()) {
        core::reporting::print_code_being_targeted(input_text);
    }

    std::expected<std::string, std::string> updated_code_or_error;

    if (configs.provider == "openai") {
        updated_code_or_error = edit_full_text_openai_(configs, input_text);
    } else {
        updated_code_or_error = edit_full_text_ollama_(configs, input_text);
    }

    if (not updated_code_or_error) {
        fmt::print("Query was rejected\n");
        return;
    }

    content.set_file_content(updated_code_or_error.value());

    if (configs.output_file) {
        fmt::print("Exported updated content to file '{}'\n", configs.output_file.value().string());
        content.export_content_to_file(configs.output_file.value());
        return;
    }

#ifndef TESTING_ENABLED
    utils::print_separator();
    char choice = 'n';

    while (true) {
        fmt::print("Overwrite file? [y/n]: ");
        choice = std::cin.get();

        if (choice == 'y' or choice == 'n') {
            break;
        } else {
            fmt::print("Invalid choice. Input either 'y' or 'n'!\n");
        }
    }

    if (choice == 'y') {
        content.export_content_to_file(configs.input_file);
    }
    utils::print_separator();
#endif
}
