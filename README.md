# Multiplayer Sandbox Game

C++20 desktop multiplayer sandbox game prototype with a headless authoritative server and an SDL3/OpenGL/Dear ImGui client.

## Project Status

| Target | Last `main` Build |
| --- | --- |
| Linux | [![Linux build status](https://img.shields.io/github/check-runs/ele-dev/multiplayer-sandbox-game/main?nameFilter=Linux%20Release&label=Linux)](https://github.com/ele-dev/multiplayer-sandbox-game/actions/workflows/build.yml?query=branch%3Amain) |
| Windows | [![Windows build status](https://img.shields.io/github/check-runs/ele-dev/multiplayer-sandbox-game/main?nameFilter=Windows%20Release&label=Windows)](https://github.com/ele-dev/multiplayer-sandbox-game/actions/workflows/build.yml?query=branch%3Amain) |

Badges track the latest OS-specific workflow check runs on `main`.

## Overview

The project explores a small server-authoritative multiplayer game architecture for desktop PC. The server runs the simulation as a headless terminal executable, while the client owns presentation, input, rendering, and debug UI.

Current prototype capabilities include local client/server networking through GameNetworkingSockets, fixed-tick authoritative movement, SDL3/OpenGL rendering, Dear ImGui overlays, and packaged Linux and Windows release artifacts from CI.

## Documentation

- [Building](BUILDING.md): local build, run, dependency, and packaging instructions.
- [Contributing](CONTRIBUTING.md): planning, branching, pull request, and coding guidelines.
- [Codebase report](docs/codebase-report.md): deeper implementation notes and architecture references.
