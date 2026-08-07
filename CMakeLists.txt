cmake_minimum_required(VERSION 3.25)

project(multiplayer_game LANGUAGES C CXX)

# Speed up build time: Enable CCache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# Speed up config time: Don't check Git remotes every re-configure
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "" FORCE)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(UNIX AND NOT APPLE)
    add_compile_definitions(_GNU_SOURCE)
    set(CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS} -D_GNU_SOURCE")
endif()

# --- Fetch Third-Party Libraries ---
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.12
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.8
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    GameNetworkingSockets
    GIT_REPOSITORY https://github.com/ValveSoftware/GameNetworkingSockets.git
    GIT_TAG v1.6.0
    GIT_SHALLOW TRUE
    GIT_SUBMODULES ""
)

# --- Dependency Flags ---
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_DIALOG OFF CACHE BOOL "" FORCE)
set(SDL_TRAY OFF CACHE BOOL "" FORCE)

set(BUILD_STATIC_LIB OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIB ON CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(ENABLE_ICE OFF CACHE BOOL "" FORCE)
set(USE_STEAMWEBRTC OFF CACHE BOOL "" FORCE)

if(WIN32)
    set(USE_CRYPTO BCrypt CACHE STRING "" FORCE)
else()
    set(USE_CRYPTO OpenSSL CACHE STRING "" FORCE)
endif()

FetchContent_MakeAvailable(
    glm
    GameNetworkingSockets
    SDL3
    imgui
)

find_package(OpenGL REQUIRED)

# ==============================================================================
#  PRECOMPILED HEADER (PCH) DEFINITIONS
# ==============================================================================

# 1. Base Core PCH: Standard Library + GLM + GameNetworkingSockets
# Used by EVERYTHING: game_shared, game_net, game_server, game_client
add_library(pch_core INTERFACE)
target_link_libraries(pch_core INTERFACE 
    glm::glm-header-only 
    GameNetworkingSockets::shared
)
target_precompile_headers(pch_core INTERFACE
    <iostream>
    <vector>
    <unordered_map>
    <string>
    <memory>
    <mutex>
    <optional>
    <atomic>
    <cstdint>
    <cstring>
    <cstddef>
    <chrono>
    <thread>
    <glm/glm.hpp>
    <glm/gtc/matrix_transform.hpp>
    <glm/gtc/type_ptr.hpp>
    <steam/steamnetworkingsockets.h>
)

# 2. Client / GUI PCH: Extends Core PCH with SDL3, OpenGL, and ImGui
# Used by: imgui target and game_client
add_library(pch_gui INTERFACE)
target_link_libraries(pch_gui INTERFACE 
    pch_core 
    SDL3::SDL3 
    OpenGL::GL
)
target_precompile_headers(pch_gui INTERFACE
    <SDL3/SDL.h>
)

# ==============================================================================
#  TARGET DEFINITIONS
# ==============================================================================

# ImGui Static Library (Uses GUI PCH for faster backend/widget compilation)
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC pch_gui)

# Game Shared (Inherits pch_core)
add_library(game_shared
    src/shared/Config.cpp
    src/shared/Simulation.cpp
)
target_include_directories(game_shared PUBLIC src)
target_link_libraries(game_shared PUBLIC pch_core)

# Game Net (Inherits pch_core via game_shared)
add_library(game_net
    src/net/GameNetworkingSocketsTransport.cpp
    src/net/Serialization.cpp
)
target_include_directories(game_net PUBLIC src)
target_link_libraries(game_net PUBLIC game_shared)

# Game Server (Inherits pch_core via game_shared/game_net)
add_executable(game_server
    src/server/main.cpp
    src/server/ServerApplication.cpp
)
target_link_libraries(game_server PRIVATE game_shared game_net)

# Game Client (Inherits pch_gui and pch_core)
add_executable(game_client
    src/client/main.cpp
    src/client/ClientApplication.cpp
    src/client/DebugOverlayLayer.cpp
    src/client/Event.cpp
    src/client/Camera.cpp
    src/client/DearImGuiContext.cpp
    src/client/graphics/DebugLineRenderer.cpp
    src/client/graphics/OpenGLFunctions.cpp
    src/client/graphics/ShaderProgram.cpp
    src/client/graphics/SolidMeshRenderer.cpp
    src/client/LayerStack.cpp
    src/client/MainMenuLayer.cpp
    src/client/OpenGLRenderer.cpp
    src/client/PauseMenuLayer.cpp
    src/client/ViewportLayer.cpp
)
target_link_libraries(game_client PRIVATE game_shared game_net imgui)

# ==============================================================================
#  HELPER FUNCTIONS & PROPERTIES
# ==============================================================================

function(target_linux_runtime_paths target_name)
    if(UNIX AND NOT APPLE)
        set_target_properties(${target_name} PROPERTIES
            BUILD_RPATH "$ORIGIN;$ORIGIN/bin;$ORIGIN/../lib"
            INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
        )
    endif()
endfunction()

function(target_strict_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

target_strict_warnings(game_shared)
target_strict_warnings(game_net)
target_strict_warnings(game_server)
target_strict_warnings(game_client)

target_linux_runtime_paths(game_server)
target_linux_runtime_paths(game_client)
if(TARGET GameNetworkingSockets)
    target_linux_runtime_paths(GameNetworkingSockets)
endif()

install(TARGETS game_client RUNTIME DESTINATION bin COMPONENT client)
install(TARGETS game_server RUNTIME DESTINATION bin COMPONENT server)