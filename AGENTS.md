# AGENTS.md

## Build System

- Use CMake as the build source of truth:
  - Linux: `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release`, then `cmake --build build --parallel`.
  - Windows: `cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -T v143 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`, then `cmake --build build --config Release --parallel`.

## Dependencies

- SDL3 is fetched by CMake with `FetchContent`; do not vendor SDL manually.
- Dear ImGui is fetched by CMake with `FetchContent` and must stay client-only.
- Avoid static linking; keep SDL3 and MSVC runtime as dynamic dependencies.

## Architecture

- The project targets desktop PC only; do not add mobile or web scaffolding.
- `game_client` owns SDL3, OpenGL, input, camera presentation, and rendering.
- `DearImGuiContext` owns Dear ImGui lifecycle and raw SDL event forwarding; GUI layers own debug overlay, crosshair, and future 2D GUI.
- `game_server` is a headless terminal executable and must not link SDL3, OpenGL, windows, or GPU APIs.
- Simulation is server-authoritative and fixed-tick; do not drive gameplay from render frame delta.
- Keep networking behind `NetworkTransport`; the active backend is standalone Valve GameNetworkingSockets.
- Keep `src/shared` independent of SDL/OpenGL and suitable for both client and server.

## CI/CD

- GitHub Actions workflow is manual (`workflow_dispatch`) plus auto on push to `main` and `develop`.
- Linux CI runs on `ubuntu-22.04` (for glibc compatibility).
- Windows CI uses explicit MSVC v143 toolset with dynamic runtime (`MultiThreadedDLL`).

## Code Guidelines

- Vertical look input is inverted at the client input layer (mouse `yrel` negated, arrow keys swapped).
- Use GLM (OpenGL Mathematics) for all vector, matrix, and math implementations across the entire codebase; avoid custom math types and functions.

## Project Management

- Active development branch is `develop`; base all future implementation tasks on the latest `develop` commit unless explicitly specified otherwise. Use GitHub MCP (`ele-dev/multiplayer-sandbox-game`) for remote pushes.
- Planning source of truth is the GitHub Project `MultiplayerGameDev` under user `ele-dev`, project number `10`; use `gh project` with the `Status` field (`Backlog`, `Ready`, `In Progress`, `In Review`, `Done`) when fetching or updating planned work.

## Autonomous Implementation Flow

- Only claim project items with `Status=Ready`.
- Choose the next task by `Priority` (`Urgent`, `High`, `Medium`, `Low`, unset) and then project order.
- Set `Status=In Progress` when claiming.
- Create a branch from latest `develop` named `agent/issue-<number>-<short-slug>`.
- Open a pull request when complete or blocked.
- Move the item to `In Review` when the PR is opened.
- Never merge automatically.

### Parallel Work with Git Worktrees

When multiple implementation agents run concurrently, use git worktrees for full isolation:

1. **Set up worktrees before spawning agents:**
   ```bash
   git worktree add .worktrees/<slug> agent/issue-<number>-<slug>
   ```
   Example:
   ```bash
   git worktree add .worktrees/feature-a    agent/issue-1-feature-a
   git worktree add .worktrees/feature-b    agent/issue-2-feature-b
   git worktree add .worktrees/feature-c    agent/issue-3-feature-c
   ```

2. **Verify setup:**
   ```bash
   git worktree list
   ```
   Confirm all worktrees are listed (main repo + each new worktree).

3. **Run each agent with its worktree as the `workdir`:**
   - Agent A → `.worktrees/feature-a/`
   - Agent B → `.worktrees/feature-b/`
   - Agent C → `.worktrees/feature-c/`

4. **Clean up after all PRs are opened:**
   ```bash
   git worktree remove .worktrees/<slug>
   ```
   Remove each worktree in any order once its PR is ready for review.
