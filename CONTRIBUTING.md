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

Use the project `Priority` field to order ready work:

- `Urgent`: take before all other ready work.
- `High`: important work that should be prioritized soon.
- `Medium`: normal priority planned work.
- `Low`: useful but not time-sensitive work.
- Unset: not yet triaged; lower priority than explicitly ranked work.

## Autonomous Agent Workflow

Autonomous coding agents should use this flow unless a task explicitly says otherwise:

- Select only project items with `Status=Ready`.
- Choose the next task by `Priority`: `Urgent`, `High`, `Medium`, `Low`, then unset.
- If multiple ready tasks have the same priority, use project order.
- If requirements are unclear before implementation starts, ask on the issue and leave the item in `Ready` or move it back to `Backlog`.
- When claiming a task, set `Status=In Progress`.
- Create a feature branch from the latest `develop` commit.
- Use branch names like `agent/issue-8-docker-server-image`.
- Implement the smallest correct change that satisfies the issue.
- Run relevant local checks before opening a PR when practical.
- Open a pull request when the task is complete or when assistance is needed.
- Move the project item to `In Review` when the PR is opened.
- Use PR comments or review replies for iterative adjustments.
- Never merge automatically; the repository owner remains the merge gate.

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
