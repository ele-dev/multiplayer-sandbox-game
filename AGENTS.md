# AGENTS.md

- Use CMake as the build source of truth: `cmake -S . -B build`, then `cmake --build build`.
- SDL3 is fetched by CMake with `FetchContent`; do not vendor SDL manually.
- The project targets desktop PC only; do not add mobile or web scaffolding.
- `game_client` owns SDL3, OpenGL, input, camera presentation, and rendering.
- `game_server` is a headless terminal executable and must not link SDL3, OpenGL, windows, or GPU APIs.
- Simulation is server-authoritative and fixed-tick; do not drive gameplay from render frame delta.
- Keep networking behind `NetworkTransport`; the first backend is UDP, with SteamNetworkingSockets planned later.
- Keep `src/shared` independent of SDL/OpenGL and suitable for both client and server.
