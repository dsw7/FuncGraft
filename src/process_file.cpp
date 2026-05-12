#include "process_file.hpp"

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "code.hpp"
#include "configs.hpp"
#include "query_classify.hpp"
#include "run_queries.hpp"
#include "utils.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <variant>

namespace {

using code::CodeToEdit;

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
    fmt::print(fg(fmt::color::gray), "Type 'q' or 'quit' to exit program.");
    fmt::print("\n");
    utils::print_separator();
}

bool validate_instructions_(const Configurations &configs, const std::string &instructions)
{
    auto classify = [&](auto &classifier, auto &response_variant) {
        using ResultsType = decltype(classifier.classify_instructions(instructions));
        ResultsType results = classifier.classify_instructions(instructions);

        if (results) {
            response_variant = *results;
        } else {
            throw std::runtime_error(results.error().errmsg);
        }
    };

    std::variant<queries::OpenAIClassification, queries::OllamaClassification> response;

    if (configs.provider == "openai") {
        queries::OpenAIClassifier classifier(configs);
        classify(classifier, response);
    } else {
        queries::OllamaClassifier classifier(configs);
        classify(classifier, response);
    }

    return std::visit([](auto &&arg) -> bool {
        if (arg.valid_instructions) {
            return true;
        }

        fmt::print(fg(fmt::color::black) | bg(fmt::color::orange_red), " Cannot process this query ");
        fmt::print("\n\n");
        fmt::print(fg(fmt::terminal_color::bright_yellow), "● {}\n", arg.reasoning);
        return false;
    },
        response);
}

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

std::string edit_text_using_llm_(
    const Configurations &configs, const std::string &instructions, const std::string &code, const std::string &language)
{
    std::variant<queries::OpenAIEdit, queries::OllamaEdit> response;

    if (configs.provider == "openai") {
        response = threading::run_openai_query(configs, instructions, code, language);
    } else {
        response = threading::run_ollama_query(configs, instructions, code, language);
    }

    return std::visit([](auto &&arg) -> std::string {
        fmt::print(fg(fmt::color::white) | bg(fmt::color::dark_golden_rod),
            " Success | Input tokens: {} | Output tokens: {} ", arg.input_tokens, arg.output_tokens);

        fmt::print("\n\n");
        fmt::print(fg(fmt::color::dim_gray), "● {}\n", arg.description_of_changes);
        utils::print_right_aligned_text(fmt::format("Total time: {}", utils::seconds_to_hhmmss(arg.total_time)));

        return arg.code;
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

void edit_file_(const Configurations &configs, const std::string &instructions)
{
    utils::print_separator();

    if (not validate_instructions_(configs, instructions)) {
        utils::print_separator();
        return;
    }

    CodeToEdit content = import_file_to_edit_(configs);
    const std::string original_code = content.get_original_code();
    const std::string language = utils::extension_to_lang(configs.input_file);
    const std::string modified_code = edit_text_using_llm_(configs, instructions, original_code, language);

    content.overwrite_original_code(modified_code);
    export_edited_file_(configs, content);
}

} // namespace

void process_file(const Configurations &configs)
{
    print_program_info_(configs);

    if (configs.instructions_from_cli) {
        edit_file_(configs, configs.instructions_from_cli.value());
        return;
    }

    if (configs.instructions_file) {
        const std::filesystem::path instructions_file = configs.instructions_file.value();
        fmt::print("Loading instructions from file '{}'\n", instructions_file.string());
        const std::string instructions = utils::read_from_file(instructions_file);
        edit_file_(configs, instructions);
        return;
    }

    std::string instructions;

    while (true) {
        fmt::print(fmt::emphasis::bold, ">>> ");
        std::getline(std::cin, instructions);

        if (instructions.empty()) {
            continue;
        } else if (instructions == "q") {
            return;
        } else if (instructions == "quit") {
            return;
        } else {
            break;
        }
    }

    edit_file_(configs, instructions);
}
