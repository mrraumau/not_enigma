cmake_minimum_required(VERSION 3.15)
project(NotEnigma LANGUAGES CXX)

if (MSVC)
    add_compile_options(/W4 /permissive- /DUNICODE /D_UNICODE)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_executable(NotEnigma WIN32
    src/main.cpp
    src/ciphers.cpp
)

target_compile_features(NotEnigma PRIVATE cxx_std_17)
