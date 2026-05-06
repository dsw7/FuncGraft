#include "process_file.hpp"

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "code.hpp"
#include "configs.hpp"
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
#include <variant>

namespace {

using adapters::OllamaResponse;
using adapters::OpenAIResponse;

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

OpenAIResponse run_openai_query_with_threading_(const Configurations &configs, const std::string &prompt)
{
    using OpenAIResults = std::expected<OpenAIResponse, adapters::OpenAIError>;

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

    OpenAIResults response = results.value();

    if (not response) {
        throw std::runtime_error(response.error().errmsg);
    }

    return *response;
}

OllamaResponse run_ollama_query_with_threading_(const Configurations &configs, const std::string &prompt)
{
    using OllamaResults = std::expected<OllamaResponse, adapters::OllamaError>;

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

    OllamaResults response = results.value();

    if (not response) {
        throw std::runtime_error(response.error().errmsg);
    }

    return *response;
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

std::optional<std::string> edit_text_using_llm_(const Configurations &configs, const std::string &prompt)
{
    std::variant<OpenAIResponse, OllamaResponse> response;

    if (configs.provider == "openai") {
        response = run_openai_query_with_threading_(configs, prompt);
    } else {
        response = run_ollama_query_with_threading_(configs, prompt);
    }

    return std::visit([](auto &&arg) -> std::optional<std::string> {
        core::reporting::print_query_info(arg);

        if (arg.was_refused) {
            return std::nullopt;
        }

        return arg.output_text;
    },
        response);
}

} // namespace

void process_file(const Configurations &configs)
{
    const std::string raw_text = utils::read_from_file(configs.input_file);
    core::code::CodeToEdit content(raw_text);
    std::string input_text = content.get_original_code();

    if (is_text_empty_(input_text)) {
        throw std::runtime_error("No code to edit");
    }

    if (content.is_delimited()) {
        core::reporting::print_code_being_targeted(input_text);
    }

    const std::string prompt = core::prompt::build_prompt(configs, input_text);

    if (configs.verbose) {
        core::reporting::print_prompt(prompt);
    }

    const std::optional<std::string> modified_text = edit_text_using_llm_(configs, prompt);

    if (not modified_text) {
        return;
    }

    content.overwrite_original_code(*modified_text);

    if (configs.output_file) {
        fmt::print("Exported updated content to file '{}'\n", configs.output_file.value().string());
        utils::write_to_file(configs.output_file.value(), content.get_modified_code());
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
        utils::write_to_file(configs.input_file, content.get_modified_code());
    }
    utils::print_separator();
#endif
}
