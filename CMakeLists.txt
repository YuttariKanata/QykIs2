cmake_minimum_required(VERSION 3.20)
project(QykIs2 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# --------------------------------------------------
# 1. 依存ライブラリ (GLFW / OpenGL)
# --------------------------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(externals/glfw)

find_package(OpenGL REQUIRED)

# --------------------------------------------------
# 2. ImGui & ImPlot ソースファイルの集約
# --------------------------------------------------
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/imgui)
set(IMPLOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/implot)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

set(IMPLOT_SOURCES
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
)

# --------------------------------------------------
# 3. ターゲット (QykIs2) の定義
# --------------------------------------------------
add_executable(QykIs2
    src/main.cpp
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
)

target_include_directories(QykIs2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMPLOT_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/externals/ac-library
)

target_link_libraries(QykIs2 PRIVATE
    glfw
    OpenGL::GL
)