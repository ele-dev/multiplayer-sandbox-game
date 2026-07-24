# Multiplayer Game

C++20 desktop multiplayer game prototype with a headless authoritative server and an SDL3/OpenGL client.

## Build

```bash
cmake -S . -B build
cmake --build build
```

On Windows multi-config generators:

```bash
cmake --build build --config Debug
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

Windows multi-config paths:

```bash
./build/Debug/game_server.exe
./build/Debug/game_client.exe
```

The prototype uses UDP on `127.0.0.1:27015`. The client opens an SDL3/OpenGL window and sends WASD/mouse-look input to the server. The server runs the authoritative fixed-tick simulation and sends snapshots back.

The client renders a debug floor grid, crosshair, and upper-right overlay with connection, tick, position, yaw/pitch, and snapshot information. Use WASD to move and mouse-look or arrow keys to rotate the camera. Press Escape to quit; the client sends a disconnect packet, and the server also times clients out after several seconds without packets.
