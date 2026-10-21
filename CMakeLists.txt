cmake_minimum_required(VERSION 3.22)

project(ArenaClash VERSION 0.1.0 LANGUAGES C CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported. Use build/desktop or build/web.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ARENA_CLASH_BUILD_APPLICATION "Build the raylib application" ON)

add_library(arena_clash_core STATIC
    src/combat/Combat.cpp
    src/core/Game.cpp
    src/input/FrameCommandBuffer.cpp
)
target_include_directories(arena_clash_core PUBLIC src)
target_compile_features(arena_clash_core PUBLIC cxx_std_17)

add_library(arena_clash_animation STATIC
    src/render/Animation.cpp
)
target_include_directories(arena_clash_animation PUBLIC src)
target_compile_features(arena_clash_animation PUBLIC cxx_std_17)
target_link_libraries(arena_clash_animation PUBLIC arena_clash_core)

if(MSVC)
    target_compile_options(arena_clash_core PRIVATE /W4 /permissive-)
    target_compile_options(arena_clash_animation PRIVATE /W4 /permissive-)
else()
    target_compile_options(arena_clash_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(arena_clash_animation PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ARENA_CLASH_BUILD_APPLICATION)
    include(FetchContent)

    if(EMSCRIPTEN)
        set(PLATFORM "Web" CACHE STRING "raylib target platform" FORCE)
    endif()

    set(BUILD_EXAMPLES OFF CACHE BOOL "Build raylib examples" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "Build raylib games" FORCE)

    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/6.2.tar.gz
        URL_HASH SHA256=df7a13129b1bec9e73a6962efff170beba310ee6eab8f94cd000b7316b4db5ee
    )
    FetchContent_MakeAvailable(raylib)

    add_executable(ArenaClash
        src/main.cpp
        src/input/KeyboardInput.cpp
        src/render/GameRenderer.cpp
    )
    target_link_libraries(ArenaClash PRIVATE arena_clash_animation raylib)
    target_include_directories(ArenaClash PRIVATE src)

    file(GLOB_RECURSE ARENA_CLASH_CHARACTER_ASSETS CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/assets/characters/*.png"
    )
    set_property(TARGET ArenaClash APPEND PROPERTY LINK_DEPENDS
        ${ARENA_CLASH_CHARACTER_ASSETS}
    )

    if(MSVC)
        target_compile_options(ArenaClash PRIVATE /W4 /permissive-)
    else()
        target_compile_options(ArenaClash PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    if(EMSCRIPTEN)
        set_target_properties(ArenaClash PROPERTIES
            OUTPUT_NAME "arena-clash"
            SUFFIX ".html"
        )
        target_link_options(ArenaClash PRIVATE
            "SHELL:-s USE_GLFW=3"
            "SHELL:-s WASM=1"
            "SHELL:-s ASSERTIONS=1"
            "SHELL:--shell-file ${CMAKE_SOURCE_DIR}/web/shell.html"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/characters@/assets/characters"
        )
    else()
        add_custom_command(TARGET ArenaClash POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_SOURCE_DIR}/assets/characters"
                    "$<TARGET_FILE_DIR:ArenaClash>/assets/characters"
            COMMENT "Copying character sprites beside ArenaClash"
        )
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(arena_clash_core_tests tests/GameTests.cpp)
    target_link_libraries(arena_clash_core_tests PRIVATE arena_clash_core)
    add_executable(arena_clash_animation_tests tests/AnimationTests.cpp)
    target_link_libraries(arena_clash_animation_tests PRIVATE arena_clash_animation)
    if(MSVC)
        target_compile_options(arena_clash_core_tests PRIVATE /W4 /permissive-)
        target_compile_options(arena_clash_animation_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(arena_clash_core_tests PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(arena_clash_animation_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME core_game_state COMMAND arena_clash_core_tests)
    add_test(NAME sprite_animation COMMAND arena_clash_animation_tests)
endif()
