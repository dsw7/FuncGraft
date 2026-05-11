#include "run_queries.hpp"

#include "query_classify.hpp"
#include "query_edit_code.hpp"

#include <array>
#include <atomic>
#include <chrono>
#include <expected>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace {

std::atomic<bool> TIMER_ENABLED_(false);

void time_api_call_()
{
    const std::chrono::duration delay = std::chrono::milliseconds(100);

    static std::array spinner = { "⠋ ", "⠙ ", "⠹ ", "⠸ ", "⠼ ", "⠴ ", "⠦ ", "⠧ ", "⠇ ", "⠏ " };
    const int num_frames = spinner.size();

#ifdef TESTING_ENABLED
    while (TIMER_ENABLED_.load()) {
        for (int i = 0; i < num_frames; ++i) {
            std::this_thread::sleep_for(delay);
        }
    }
#else
    while (TIMER_ENABLED_.load()) {
        for (int i = 0; i < num_frames; ++i) {
            std::cout << " \r" << spinner[i] << std::flush;
            std::this_thread::sleep_for(delay);
        }
    }

    std::cout << " \r" << std::flush;
#endif
}

} // namespace

namespace core {
namespace threading {

adapters::OpenAIClassification classify_instructions_openai(const Configurations &configs, const std::string &prompt)
{
    using OpenAIResults = std::expected<adapters::OpenAIClassification, adapters::OpenAIError>;

    OpenAIResults results = queries::OpenAIClassifier(configs).classify_instructions(prompt);

    if (not results) {
        throw std::runtime_error(results.error().errmsg);
    }
    return *results;
}

adapters::OllamaClassification classify_instructions_ollama(const Configurations &configs, const std::string &prompt)
{
    using OllamaResults = std::expected<adapters::OllamaClassification, adapters::OllamaError>;

    OllamaResults results = queries::OllamaClassifier(configs).classify_instructions(prompt);

    if (not results) {
        throw std::runtime_error(results.error().errmsg);
    }
    return *results;
}

adapters::OpenAIEdit run_openai_query(const Configurations &configs, const std::string &prompt)
{
    using OpenAIResults = std::expected<adapters::OpenAIEdit, adapters::OpenAIError>;

    TIMER_ENABLED_.store(true);
    std::thread timer(time_api_call_);

    std::optional<OpenAIResults> results;
    bool query_failed = false;
    std::string errmsg;

    try {
        results = adapters::OpenAI(configs).query_edit_code(prompt);
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

adapters::OllamaEdit run_ollama_query(const Configurations &configs, const std::string &prompt)
{
    using OllamaResults = std::expected<adapters::OllamaEdit, adapters::OllamaError>;

    TIMER_ENABLED_.store(true);
    std::thread timer(time_api_call_);

    std::optional<OllamaResults> results;
    bool query_failed = false;
    std::string errmsg;

    try {
        results = queries::OllamaCodeEditor(configs).edit_code(prompt);
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

} // namespace threading
} // namespace core
