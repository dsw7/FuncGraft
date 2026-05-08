#include "process_file.hpp"

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "code.hpp"
#include "configs.hpp"
#include "prompt.hpp"
#include "reporting.hpp"
#include "utils.hpp"

#include <array>
#include <atomic>
#include <chrono>
#include <expected>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <thread>
#include <variant>

namespace {

using adapters::OllamaResponse;
using adapters::OpenAIResponse;
using core::code::CodeToEdit;

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

// Steps ----------------------------------------------------------------------------------------------------

CodeToEdit import_file_to_edit_(const Configurations &configs)
{
    const std::string raw_text = utils::read_from_file(configs.input_file);

    CodeToEdit content(raw_text);

    if (content.is_delimited()) {
        core::reporting::print_code_being_targeted(content.get_original_code());
    }

    return content;
}

std::string load_instructions_(const Configurations &configs)
{
    if (configs.instructions_from_cli) {
        return configs.instructions_from_cli.value();
    }

    if (configs.instructions_file) {
        const std::filesystem::path instructions_file = configs.instructions_file.value();
        fmt::print("Loading instructions from file '{}'\n", instructions_file.string());
        return utils::read_from_file(instructions_file);
    }

    utils::print_separator();
    std::string instructions;

    while (true) {
        fmt::print(fmt::emphasis::bold, "> ");
        std::getline(std::cin, instructions);

        if (not instructions.empty()) {
            break;
        }
    }

    utils::print_separator();
    return instructions;
}

std::string create_prompt_(const Configurations &configs, const CodeToEdit &content, const std::string &instructions)
{
    const std::string original_code = content.get_original_code();
    const std::string prompt = core::prompt::build_prompt(configs, instructions, original_code);

    if (configs.verbose) {
        core::reporting::print_prompt(prompt);
    }

    return prompt;
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

void export_edited_file_(const Configurations &configs, const CodeToEdit &content)
{
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

} // namespace

void process_file(const Configurations &configs)
{
    core::reporting::print_program_info(configs);
    CodeToEdit content = import_file_to_edit_(configs);
    const std::string instructions = load_instructions_(configs);
    const std::string prompt = create_prompt_(configs, content, instructions);

    const std::optional<std::string> modified_code = edit_text_using_llm_(configs, prompt);
    if (not modified_code) {
        utils::print_separator();
        return;
    }

    content.overwrite_original_code(*modified_code);
    export_edited_file_(configs, content);
}
