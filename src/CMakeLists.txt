cmake_minimum_required(VERSION 3.18)
project(GPTifierEdit VERSION 1.0)

option(ENABLE_TESTING "Set the TESTING_ENABLED macro" OFF)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local/bin")
set(SRC_FILES
  curl_base.cpp
  file_io.cpp
  instructions.cpp
  main.cpp
  params.cpp
  process_file.cpp
  prompt.cpp
  query_llm.cpp
  text_manip.cpp
  utils.cpp
)

add_compile_options(-Wall -Wextra -pedantic -Werror)

string(TIMESTAMP CURRENT_YEAR "%Y")
add_compile_definitions(CURRENT_YEAR="${CURRENT_YEAR}")

if(ENABLE_TESTING)
  add_compile_definitions(TESTING_ENABLED)
endif()

if(ENABLE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

include_directories(external)

# -----------------------------------------------------------------------------------------------------------
# External dependencies

find_package(fmt REQUIRED)

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
add_executable(edit ${SRC_FILES})

target_link_libraries(edit fmt::fmt curl)
install(TARGETS edit DESTINATION ${CMAKE_INSTALL_PREFIX})
