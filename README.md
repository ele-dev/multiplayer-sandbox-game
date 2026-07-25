# Multiplayer Game

C++20 desktop multiplayer game prototype with a headless authoritative server and an SDL3/OpenGL/Dear ImGui client.

## Build

Linux Release build, matching the CI workflow:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
BUILD_JOBS=$(nproc); BUILD_JOBS=$(( BUILD_JOBS > 1 ? BUILD_JOBS / 2 : 1 ))
cmake --build build --parallel "$BUILD_JOBS"
```

Windows Release build with Visual Studio 2022 MSVC v143, matching the CI workflow:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -T v143 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL
$buildJobs = [Math]::Max(1, [Math]::Floor([Environment]::ProcessorCount / 2))
cmake --build build --config Release --parallel $buildJobs
```

## Run

Start the server first:

```bash
./build/game_server
```

Then start the client in another terminal:

```bash
./build/game_client
```

Windows Release paths:

```powershell
./build/Release/game_server.exe
./build/Release/game_client.exe
```

The prototype uses UDP on `127.0.0.1:27015`. The client opens an SDL3/OpenGL window and sends WASD/mouse-look input to the server. The server runs the authoritative fixed-tick simulation and sends snapshots back.

The client renders a debug floor grid, Dear ImGui crosshair, and upper-right Dear ImGui overlay with connection, tick, position, yaw/pitch, and snapshot information. Use WASD to move and mouse-look or arrow keys to rotate the camera. Press Escape to quit; the client sends a disconnect packet, and the server also times clients out after several seconds without packets.
