#include "process_file.hpp"

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "code.hpp"
#include "configs.hpp"
#include "prompt.hpp"
#include "run_queries.hpp"
#include "utils.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <variant>

namespace {

void print_program_info_(const Configurations &configs)
{
    fmt::print("● FuncGraft ");
    fmt::print(fg(fmt::color::gray), "v{}\n", PROJECT_VERSION);

    if (configs.provider) {
        if (*configs.provider == "ollama") {
            fmt::print("● Ollama · {}\n", configs.model_ollama);
            fmt::print(fg(fmt::color::gray), "  ⎿ {}:{}\n", configs.host_ollama, configs.port_ollama);
        } else {
            fmt::print("● OpenAI · {}\n", configs.model_openai);
        }
    }

    fmt::print("● ");
    fmt::print(fg(fmt::color::yellow_green), "{}\n\n", configs.input_file.string());
}

using core::code::CodeToEdit;

CodeToEdit import_file_to_edit_(const Configurations &configs)
{
    const std::string raw_text = utils::read_from_file(configs.input_file);

    CodeToEdit content(raw_text);

    if (content.is_delimited()) {
        utils::print_separator();
        fmt::print(fg(fmt::color::dim_gray), "@@@\n");
        fmt::print(fg(fmt::terminal_color::bright_blue), "{}", content.get_original_code());
        fmt::print(fg(fmt::color::dim_gray), "@@@\n");
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
        fmt::print(fmt::emphasis::bold, "Prompt:\n");
        fmt::print(fg(fmt::terminal_color::bright_blue), "{}", prompt);
        utils::print_separator();
    }

    return prompt;
}

template<typename T>
void print_query_info_(const T &response)
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

    utils::print_right_aligned_text(
        fmt::format("Total time: {}", utils::seconds_to_hhmmss(response.total_time)));
}

std::optional<std::string> edit_text_using_llm_(const Configurations &configs, const std::string &prompt)
{
    std::variant<adapters::OpenAIResponse, adapters::OllamaResponse> response;

    if (configs.provider == "openai") {
        response = core::threading::run_openai_query(configs, prompt);
    } else {
        response = core::threading::run_ollama_query(configs, prompt);
    }

    return std::visit([](auto &&arg) -> std::optional<std::string> {
        print_query_info_(arg);

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
    print_program_info_(configs);

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
