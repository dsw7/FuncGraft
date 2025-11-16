#include "process_file.hpp"

#include "file_io.hpp"
#include "instructions.hpp"
#include "prompt.hpp"
#include "query_openai.hpp"
#include "utils.hpp"

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
fmt::terminal_color green = fmt::terminal_color::bright_green;

// Threading ------------------------------------------------------------------------------------------------

std::atomic<bool> TIMER_ENABLED(false);

void time_api_call()
{
    const std::chrono::duration delay = std::chrono::milliseconds(25);
    int counter = 0;

    while (TIMER_ENABLED.load()) {
        switch (counter) {
            case 0:
                std::cout << ">        \r" << std::flush;
                break;
            case 5:
                std::cout << "->       \r" << std::flush;
                break;
            case 10:
                std::cout << "-->      \r" << std::flush;
                break;
            case 15:
                std::cout << "--->     \r" << std::flush;
                break;
            case 20:
                std::cout << "---->    \r" << std::flush;
                break;
            case 25:
                std::cout << "----->   \r" << std::flush;
                break;
            case 30:
                std::cout << "------>  \r" << std::flush;
                break;
            case 35:
                std::cout << "-------> \r" << std::flush;
                break;
            case 40:
                std::cout << "-------->\r" << std::flush;
                break;
        }
        counter++;

        if (counter > 44) {
            counter = 0;
        }

        std::this_thread::sleep_for(delay);
    }

    std::cout << std::string(16, ' ') << '\r' << std::flush;
}

query_openai::QueryResults run_query_with_threading(const std::string &prompt, const std::string &model)
{
    TIMER_ENABLED.store(true);
    std::thread timer(time_api_call);

    query_openai::QueryResults results;
    bool query_failed = false;
    std::string errmsg;

    try {
        results = query_openai::run_query(prompt, model);
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

void print_prompt_if_verbose(const std::string &prompt)
{
    utils::print_separator();
    fmt::print(fmt::emphasis::bold, "Prompt:\n");
    fmt::print(fg(blue), "{}", prompt);
}

void report_information_about_query(const query_openai::QueryResults &results)
{
    fmt::print(fmt::emphasis::bold, "Information:\n");
    fmt::print("Prompt tokens: {}\n", results.prompt_tokens);
    fmt::print("Completion tokens: {}\n", results.completion_tokens);
    fmt::print("Description of changes: ");
    fmt::print(fg(blue), "{}\n", results.description);
}

std::string edit_delimited_text(const params::CommandLineParameters &params, const std::string &input_text)
{
    file_io::Parts text_parts = file_io::unpack_text_into_parts(input_text);

    const std::string instructions = instructions::load_instructions(params);
    const std::string prompt = prompt::build_prompt(instructions, text_parts.core, params.input_file.extension());

    if (params.verbose) {
        print_prompt_if_verbose(prompt);
    }
    utils::print_separator();

    const query_openai::QueryResults results = run_query_with_threading(prompt, params.model);
    report_information_about_query(results);

    text_parts.core = results.output_text;
    return file_io::pack_parts_into_text(text_parts);
}

std::string edit_full_text(const params::CommandLineParameters &params, const std::string &input_text)
{
    const std::string instructions = instructions::load_instructions(params);
    const std::string prompt = prompt::build_prompt(instructions, input_text, params.input_file.extension());

    if (params.verbose) {
        print_prompt_if_verbose(prompt);
    }
    utils::print_separator();

    const query_openai::QueryResults results = run_query_with_threading(prompt, params.model);
    report_information_about_query(results);

    return results.output_text;
}

void print_updated_code_to_stdout(const std::string &code, const std::filesystem::path &input_file)
{
    const auto label = utils::resolve_label_from_extension(input_file.extension());
    std::string code_block;

    if (label) {
        code_block = utils::get_code_block(code, label.value());
    } else {
        code_block = utils::get_code_block(code);
    }

    fmt::print(fmt::emphasis::bold, "Results:\n");
    fmt::print(fg(green), "{}", code_block);

#ifndef TESTING_ENABLED
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
        utils::write_to_file(input_file, code);
    }
#endif
}

} // namespace

namespace process_file {

void process_file(const params::CommandLineParameters &params)
{
    const std::string input_text = file_io::load_input_text(params.input_file);
    std::string output_text;

    if (file_io::is_text_delimited(input_text)) {
        output_text = edit_delimited_text(params, input_text);
    } else {
        output_text = edit_full_text(params, input_text);
    }

    if (params.output_file) {
        fmt::print("Exported updated content to file '{}'\n", params.output_file.value().string());
        file_io::write_output_text(params.output_file.value(), output_text);
        return;
    }

    utils::print_separator();
    print_updated_code_to_stdout(output_text, params.input_file);
    utils::print_separator();
}

} // namespace process_file
