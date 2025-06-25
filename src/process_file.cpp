#include "process_file.hpp"

#include "file_io.hpp"
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

std::string load_instructions(const params::CommandLineParameters &params)
{
    if (params.instructions_from_cli) {
        fmt::print("Loading instructions from command line\n");
        return params.instructions_from_cli.value();
    }

    if (not params.instructions_file) {
        throw std::runtime_error("No instructions provided by file or command line");
    }

    const std::string instructions_file = params.instructions_file.value().string();

    if (not std::filesystem::exists(instructions_file)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", instructions_file));
    }

    fmt::print("Loading instructions from file '{}'\n", instructions_file);
    return utils::read_from_file(instructions_file);
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

void report_information_about_query(const query_openai::QueryResults &results)
{
    fmt::print(fmt::emphasis::bold, "Information:\n");
    fmt::print("Prompt tokens: {}\n", results.prompt_tokens);
    fmt::print("Completion tokens: {}\n", results.completion_tokens);
    fmt::print("Description of changes: ");
    fmt::print(fg(blue), "{}\n", results.description);
}

} // namespace

namespace process_file {

void process_file(const params::CommandLineParameters &params)
{
    utils::print_separator();

    const std::string instructions = load_instructions(params);

    if (instructions.empty()) {
        throw std::runtime_error("Instructions are empty!");
    }

    file_io::FileIO target;
    fmt::print("Loading contents from file '{}'\n", params.input_file.string());
    target.load_input_text_from_file(params.input_file);

    const std::string input_text = target.get_text();
    const std::string prompt = prompt::build_prompt(instructions, input_text, params.input_file.extension());

    fmt::print("Using model: {}\n", params.model);

    if (params.verbose) {
        utils::print_separator();
        fmt::print(fmt::emphasis::bold, "Prompt:\n");
        fmt::print(fg(blue), "{}", prompt);
    }

    utils::print_separator();
    const query_openai::QueryResults results = run_query_with_threading(prompt, params.model);

    target.set_text(results.output_text);
    report_information_about_query(results);

    if (params.output_file) {
        fmt::print("Exported updated content to file '{}'\n", params.output_file.value().string());
        target.dump_output_text_to_file(params.output_file.value());
        return;
    }

    utils::print_separator();
    print_updated_code_to_stdout(target.dump_output_text_to_string(), params.input_file);
    utils::print_separator();
}

} // namespace process_file
