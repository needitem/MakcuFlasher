cmake_minimum_required(VERSION 3.10)
project(MakcuFlasher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # Statically link the MSVC runtime so the second PC
    # does not need a separate VC++ Redistributable installed.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_executable(MakcuFlasher
    src/flasher_main.cpp
    src/MakcuFlasher.cpp
)

target_include_directories(MakcuFlasher PRIVATE src)

if (WIN32)
    # Windows doesn't need additional libs for serial I/O
else()
    # Linux: link pthread for std::thread
    target_link_libraries(MakcuFlasher PRIVATE pthread)
endif()
