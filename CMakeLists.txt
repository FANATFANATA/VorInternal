cmake_minimum_required(VERSION 3.20)
project(VorInternal CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "CS2 is x64 only. VorInternal and VorInjector must be compiled as x64. Please use '-A x64' with CMake.")
endif()

add_library(VorInternal SHARED
    src/dllmain.cpp
    src/config/config.cpp
    src/cheat/cheat.cpp
    src/render/render.cpp
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/backends/imgui_impl_dx11.cpp
    src/imgui/backends/imgui_impl_win32.cpp
    src/minhook/src/buffer.c
    src/minhook/src/hook.c
    src/minhook/src/trampoline.c
    src/minhook/src/hde/hde64.c
)

target_include_directories(VorInternal PRIVATE
    src
    src/imgui
    src/imgui/backends
    src/minhook/include
)

target_link_libraries(VorInternal PRIVATE d3d11.lib dxgi.lib)

add_executable(VorInjector src/injector.cpp)
target_link_libraries(VorInjector PRIVATE kernel32.lib user32.lib)
