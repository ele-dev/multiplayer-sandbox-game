# AGENTS.md

- Use CMake as the build source of truth: `cmake -S . -B build`, then `cmake --build build`.
- SDL3 is fetched by CMake with `FetchContent`; do not vendor SDL manually.
- Dear ImGui is fetched by CMake with `FetchContent` and must stay client-only.
- The project targets desktop PC only; do not add mobile or web scaffolding.
- `game_client` owns SDL3, OpenGL, input, camera presentation, and rendering.
- `GuiLayer` owns Dear ImGui lifecycle, event forwarding, debug overlay, crosshair, and future 2D GUI.
- `game_server` is a headless terminal executable and must not link SDL3, OpenGL, windows, or GPU APIs.
- Simulation is server-authoritative and fixed-tick; do not drive gameplay from render frame delta.
- Keep networking behind `NetworkTransport`; the first backend is UDP, with SteamNetworkingSockets planned later.
- Keep `src/shared` independent of SDL/OpenGL and suitable for both client and server.
