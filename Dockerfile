# syntax=docker/dockerfile:1

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        libssl3 \
        libstdc++6 \
        zlib1g \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd --gid 1000 game-server \
    && useradd --uid 1000 --gid game-server --no-create-home \
        --home-dir /app --shell /usr/sbin/nologin game-server

WORKDIR /app

COPY --chown=game-server:game-server --chmod=0555 dist/linux/game_server ./game_server
COPY --chown=game-server:game-server --chmod=0555 dist/linux/lib/ ./lib/

ENV LD_LIBRARY_PATH=/app/lib

USER game-server

EXPOSE 27015/udp

ENTRYPOINT ["./game_server"]
