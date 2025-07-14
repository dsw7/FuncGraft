cmake_minimum_required(VERSION 3.18)

project(GPTifierEdit VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

string(TIMESTAMP BUILD_DATE)
add_compile_definitions(BUILD_DATE="${BUILD_DATE}")

add_compile_options(-Wall -Wextra -pedantic -Werror)

# Need to generate compile_commands.json for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------------------------------------
# Testing

option(ENABLE_TESTING "Set the TESTING_ENABLED macro" OFF)

if(ENABLE_TESTING)
  add_compile_definitions(TESTING_ENABLED)
endif()

# -----------------------------------------------------------------------------------------------------------
# nlohmann/json.hpp

message(STATUS "Checking for nlohmann/json.hpp")
set(NLOHMANN_JSON_HPP_DL "${CMAKE_SOURCE_DIR}/external/json.hpp")

if(EXISTS ${NLOHMANN_JSON_HPP_DL})
  message(STATUS "${NLOHMANN_JSON_HPP_DL} already exists. Skipping download")
else()
  message(STATUS "Downloading nlohmann/json.hpp")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp"
    ${NLOHMANN_JSON_HPP_DL}
    SHOW_PROGRESS
  )
endif()

# -----------------------------------------------------------------------------------------------------------
# toml++

message(STATUS "Checking for toml++ header file")
set(TOMLPLUSPLUS_HPP_DL "${CMAKE_SOURCE_DIR}/external/toml.hpp")

if(EXISTS ${TOMLPLUSPLUS_HPP_DL})
  message(STATUS "${TOMLPLUSPLUS_HPP_DL} already exists. Skipping download")
else()
  message(STATUS "Downloading toml.hpp")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/marzer/tomlplusplus/master/toml.hpp"
    ${TOMLPLUSPLUS_HPP_DL}
    SHOW_PROGRESS
  )
endif()

# -----------------------------------------------------------------------------------------------------------
find_package(fmt REQUIRED)
include_directories(external)

add_executable(
  edit
  curl_base.cpp
  file_io.cpp
  instructions.cpp
  main.cpp
  process_file.cpp
  prompt.cpp
  query_openai.cpp
  utils.cpp
)

target_link_libraries(edit fmt::fmt curl)

set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local")
install(TARGETS edit DESTINATION bin)
