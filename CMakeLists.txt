cmake_minimum_required(VERSION 3.22.1)
project(runner LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.x
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(SFML)

file(GLOB_RECURSE SOURCES
    src/*.mm
    src/*.m
    src/*.cpp
    src/*.c
    src/*.cxx
    src/*.h
)

add_executable(runner ${SOURCES})
target_include_directories(runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(runner PRIVATE sfml-graphics sfml-audio sfml-network)
target_compile_features(runner PRIVATE cxx_std_17)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(runner PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(runner PRIVATE /W4)
endif()

add_custom_command(
    TARGET runner
    COMMENT "Copy assets directory"
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:runner>/assets
    VERBATIM)
