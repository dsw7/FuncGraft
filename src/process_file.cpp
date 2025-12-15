#include "process_file.hpp"

#include "file_io.hpp"
#include "instructions.hpp"
#include "prompt.hpp"
#include "query_llm.hpp"
#include "text_manip.hpp"
#include "utils.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace {

fmt::terminal_color blue = fmt::terminal_color::bright_blue;

// Threading ------------------------------------------------------------------------------------------------

std::atomic<bool> TIMER_ENABLED(false);

void print_progress(const int n)
{
    std::string progress = "         \r";
    static int size_p = progress.size();
    const int n_c = std::clamp(n, 0, size_p);

    for (int i = 0; i < n_c; i++) {
        progress[i] = '-';
    }
    progress[n_c] = '>';

    std::cout << progress << std::flush;
}

void time_api_call()
{
    const std::chrono::duration delay = std::chrono::milliseconds(25);
    int counter = 0;

    while (TIMER_ENABLED.load()) {
        if (counter % 5 == 0) {
            print_progress(counter / 5);
        }
        counter++;

        if (counter > 44) {
            counter = 0;
        }
        std::this_thread::sleep_for(delay);
    }

    std::cout << std::string(16, ' ') << '\r' << std::flush;
}

query_llm::ResultsOpenAI run_openai_query_with_threading(const std::string &prompt, const std::string &model)
{
    TIMER_ENABLED.store(true);
    std::thread timer(time_api_call);

    query_llm::ResultsOpenAI results;
    bool query_failed = false;
    std::string errmsg;

    try {
        results = query_llm::run_openai_query(prompt, model);
    } catch (std::runtime_error &e) {
        errmsg = e.what();
        query_failed = true;
    }

    TIMER_ENABLED.store(false);
    timer.join();

    if (query_failed) {
        throw std::runtime_error(errmsg);
    }

    return results;
}

// ----------------------------------------------------------------------------------------------------------

void print_code_being_targeted(const std::string &code)
{
    utils::print_separator();
    fmt::print(fmt::emphasis::bold, "Delimited code:\n");
    fmt::print(fg(blue), "{}", code);
}

void print_prompt_if_verbose(const std::string &prompt)
{
    utils::print_separator();
    fmt::print(fmt::emphasis::bold, "Prompt:\n");
    fmt::print(fg(blue), "{}", prompt);
}

void report_information_about_query(const query_llm::ResultsOpenAI &results)
{
    utils::print_separator();
    fmt::print(fmt::emphasis::bold, "Information:\n");
    fmt::print("Input tokens: {}\n", results.input_tokens);
    fmt::print("Output tokens: {}\n", results.output_tokens);
    fmt::print("Description of changes: ");
    fmt::print(fg(blue), "{}\n", results.description);
}

std::string edit_delimited_text(const params::CommandLineParameters &params, const std::string &input_text)
{
    text_manip::Parts text_parts = text_manip::unpack_text_into_parts(input_text);

    if (text_manip::is_text_empty(text_parts.original_text)) {
        throw std::runtime_error("The delimited block does not contain any code");
    }

    print_code_being_targeted(text_parts.original_text);

    const std::string instructions = instructions::load_instructions(params);
    const std::string prompt = prompt::build_openai_prompt(instructions, text_parts.original_text, params.input_file.extension());

    if (params.verbose) {
        print_prompt_if_verbose(prompt);
    }

    const query_llm::ResultsOpenAI results = run_openai_query_with_threading(prompt, params.model);
    report_information_about_query(results);

    text_parts.modified_text = results.output_text;
    return text_manip::pack_parts_into_text(text_parts);
}

std::string edit_full_text(const params::CommandLineParameters &params, const std::string &input_text)
{
    const std::string instructions = instructions::load_instructions(params);
    const std::string prompt = prompt::build_openai_prompt(instructions, input_text, params.input_file.extension());

    if (params.verbose) {
        print_prompt_if_verbose(prompt);
    }

    const query_llm::ResultsOpenAI results = run_openai_query_with_threading(prompt, params.model);
    report_information_about_query(results);

    return results.output_text;
}

} // namespace

namespace process_file {

void process_file(const params::CommandLineParameters &params)
{
    const std::string input_text = file_io::import_file(params.input_file);

    if (text_manip::is_text_empty(input_text)) {
        throw std::runtime_error("The file does not contain any code");
    }

    std::string output_text;

    if (text_manip::is_text_delimited(input_text)) {
        output_text = edit_delimited_text(params, input_text);
    } else {
        output_text = edit_full_text(params, input_text);
    }

    if (params.output_file) {
        fmt::print("Exported updated content to file '{}'\n", params.output_file.value().string());
        file_io::export_file(output_text, params.output_file.value());
        return;
    }

#ifndef TESTING_ENABLED
    utils::print_separator();
    file_io::export_file_with_prompt(output_text, params.input_file);
    utils::print_separator();
#endif
}

} // namespace process_file
