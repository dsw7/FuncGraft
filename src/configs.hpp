#pragma once

#include <string>

struct Configs {
    void load_configs_from_config_file();

    int port_ollama = -1;
    std::string host_ollama;
    std::string model_ollama;
    std::string model_openai;
};

extern Configs configs;
