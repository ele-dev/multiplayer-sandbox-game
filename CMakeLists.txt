cmake_minimum_required(VERSION 3.25)

project(multiplayer_game LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
)

set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
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

FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(GameNetworkingSockets)
FetchContent_MakeAvailable(SDL3)
FetchContent_MakeAvailable(imgui)

find_package(OpenGL REQUIRED)

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
target_link_libraries(imgui PUBLIC SDL3::SDL3 OpenGL::GL)

add_library(game_shared
    src/shared/Config.cpp
    src/shared/Simulation.cpp
)
target_include_directories(game_shared PUBLIC src)
target_link_libraries(game_shared PUBLIC glm)

add_library(game_net
    src/net/GameNetworkingSocketsTransport.cpp
    src/net/Serialization.cpp
)
target_link_libraries(game_net PUBLIC game_shared GameNetworkingSockets::shared)
target_include_directories(game_net PUBLIC src)

add_executable(game_server
    src/server/main.cpp
    src/server/ServerApplication.cpp
)
target_link_libraries(game_server PRIVATE game_shared game_net)

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
target_link_libraries(game_client PRIVATE game_shared game_net SDL3::SDL3 OpenGL::GL imgui)

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
