# Contributing

Thanks for contributing to the Multiplayer Sandbox Game prototype.

## Planning

Planned work is tracked in the GitHub Project `MultiplayerGameDev` under user `ele-dev`, project number `10`.

Use the project `Status` field consistently:

- `Backlog`: captured ideas, bugs, and future work that are not ready to start.
- `Ready`: scoped work that can be picked up.
- `In Progress`: active implementation work.
- `In Review`: work with an open pull request or pending review.
- `Done`: completed and verified work.

## Branching

Base implementation work on the latest `develop` commit unless the task explicitly says otherwise.

Use focused feature branches for non-trivial changes. Keep commits small and tied to the task being solved.

## Pull Requests

Before opening a pull request:

- Build locally when practical.
- Run the relevant CMake build command from `BUILDING.md`.
- Keep unrelated file changes out of the branch.
- Include the linked issue or project task in the PR description when applicable.

## Coding Guidelines

- Keep `game_server` headless; it must not link SDL3, OpenGL, windows, or GPU APIs.
- Keep `src/shared` independent of SDL/OpenGL and suitable for client and server use.
- Keep networking behind `NetworkTransport`.
- Use GLM for vector, matrix, and math implementations.
- Do not vendor SDL3, Dear ImGui, GLM, or GameNetworkingSockets manually.
- Preserve the fixed-tick, server-authoritative simulation model.

## Continuous Integration

The `Build` GitHub Actions workflow runs automatically on pushes to `main` and `develop` and can also be started manually with `workflow_dispatch`.

The workflow builds Linux and Windows release artifacts. Windows uses vcpkg binary caching to avoid rebuilding Protobuf from source when the cache is hot.
