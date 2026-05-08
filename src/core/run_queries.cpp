#include "run_queries.hpp"

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

    while (TIMER_ENABLED_.load()) {
        for (int i = 0; i < num_frames; ++i) {
            std::cout << " \r" << spinner[i] << std::flush;
            std::this_thread::sleep_for(delay);
        }
    }

    std::cout << " \r" << std::flush;
}

} // namespace

namespace core {
namespace threading {

adapters::OpenAIResponse run_openai_query(const Configurations &configs, const std::string &prompt)
{
    using OpenAIResults = std::expected<adapters::OpenAIResponse, adapters::OpenAIError>;

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

adapters::OllamaResponse run_ollama_query(const Configurations &configs, const std::string &prompt)
{
    using OllamaResults = std::expected<adapters::OllamaResponse, adapters::OllamaError>;

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

} // namespace threading
} // namespace core
