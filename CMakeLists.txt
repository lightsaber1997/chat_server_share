cmake_minimum_required(VERSION 3.15)
project(ChatApp LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Root sources
set(SOURCES
    Main.cpp
    ChatManager.cpp
    PromptHandler.cpp
)

set(HEADERS
    PromptHandler.hpp
    ChatManager.hpp
    json.hpp   # header-only JSON, included for IDE visibility
)

add_executable(ChatApp ${SOURCES} ${HEADERS})

# Expect QNN_SDK_ROOT to be set in environment
if(NOT DEFINED ENV{QNN_SDK_ROOT})
    message(FATAL_ERROR "QNN_SDK_ROOT environment variable not set")
endif()

set(QNN_SDK_ROOT $ENV{QNN_SDK_ROOT})

# Include paths (Genie headers + project root for json.hpp)
target_include_directories(ChatApp PRIVATE
    ${QNN_SDK_ROOT}/include/Genie
    ${CMAKE_SOURCE_DIR}   # so we can #include "json.hpp"
)

# Link paths and libs
target_link_directories(ChatApp PRIVATE
    ${QNN_SDK_ROOT}/lib/aarch64-windows-msvc
)

target_link_libraries(ChatApp PRIVATE
    Genie
)

# MSVC-specific flags
if(MSVC)
    target_compile_options(ChatApp PRIVATE /utf-8)
endif()

# Post-build: copy runtime DLLs
if(WIN32)
    add_custom_command(TARGET ChatApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${QNN_SDK_ROOT}/lib/hexagon-v73/unsigned
                $<TARGET_FILE_DIR:ChatApp>
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${QNN_SDK_ROOT}/lib/aarch64-windows-msvc
                $<TARGET_FILE_DIR:ChatApp>
    )
endif()
