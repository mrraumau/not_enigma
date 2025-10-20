cmake_minimum_required(VERSION 3.15)
project(NotEnigma LANGUAGES CXX)

if (MSVC)
    add_compile_options(/W4 /permissive- /DUNICODE /D_UNICODE)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_library(NotEnigmaCiphers STATIC
    src/ciphers.cpp
)
target_include_directories(NotEnigmaCiphers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_features(NotEnigmaCiphers PUBLIC cxx_std_17)

add_executable(NotEnigmaConsole
    src/console_main.cpp
)
target_link_libraries(NotEnigmaConsole PRIVATE NotEnigmaCiphers)
target_compile_features(NotEnigmaConsole PRIVATE cxx_std_17)

option(NOT_ENIGMA_BUILD_TESTS "Build console cipher tests" ON)
if (NOT_ENIGMA_BUILD_TESTS)
    enable_testing()
    add_executable(NotEnigmaCipherTests
        tests/cipher_tests.cpp
    )
    target_link_libraries(NotEnigmaCipherTests PRIVATE NotEnigmaCiphers)
    target_compile_features(NotEnigmaCipherTests PRIVATE cxx_std_17)
    add_test(NAME cipher_tests COMMAND NotEnigmaCipherTests)
endif()

option(NOT_ENIGMA_BUILD_GUI "Build the Win32 GUI application" ${WIN32})
if (NOT_ENIGMA_BUILD_GUI)
    if (WIN32)
        add_executable(NotEnigma WIN32
            src/main.cpp
        )
        target_link_libraries(NotEnigma PRIVATE NotEnigmaCiphers)
        target_compile_features(NotEnigma PRIVATE cxx_std_17)
    else()
        message(WARNING "GUI build requested but WIN32 platform not detected; skipping GUI target")
    endif()
endif()
