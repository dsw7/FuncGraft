#pragma once

#include <string>

struct Configs {
    void load_configs_from_config_file();

    std::string model_ollama;
    std::string model_openai;
};

extern Configs configs;
