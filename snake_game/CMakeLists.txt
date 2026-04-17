cmake_minimum_required(VERSION 3.16)
project(SnakeGame VERSION 1.0 LANGUAGES CXX)

# ── C++17 bắt buộc (dùng std::optional, std::thread...) ──────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ── Tìm SFML 3.x ─────────────────────────────────────────────
# SFML 3 đổi tên component: không còn "window" riêng, Graphics kéo theo tất cả
find_package(SFML 3 COMPONENTS Graphics REQUIRED)

# ── Source file ───────────────────────────────────────────────
add_executable(${PROJECT_NAME} main.cpp)

# ── Link SFML 3 ───────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics)

# Threads (std::thread dùng trong logicUpdate)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# ── Copy arial.ttf sang build folder ─────────────────────────
if(EXISTS "${CMAKE_SOURCE_DIR}/arial.ttf")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/arial.ttf"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/arial.ttf"
        COMMENT "Copying arial.ttf to build directory"
    )
else()
    message(WARNING
        "Khong tim thay arial.ttf!\n"
        "  -> Dat file arial.ttf vao cung thu muc CMakeLists.txt\n"
        "  -> Hoac sua duong dan trong initUI() cua main.cpp"
    )
endif()

# ── Compiler warnings ─────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Windows: copy SFML DLLs sang build folder ────────────────
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()
