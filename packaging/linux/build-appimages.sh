#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="${BUILD_DIR:-build}"
output_dir="${OUTPUT_DIR:-dist/linux}"
tools_dir="${APPIMAGE_TOOLS_DIR:-${repo_root}/.cache/appimage-tools}"
arch="${APPIMAGE_ARCH:-x86_64}"
branch="${APPIMAGE_BRANCH:-${GITHUB_REF_NAME:-}}"

if [[ -z "${branch}" ]]; then
    branch="$(git -C "${repo_root}" rev-parse --abbrev-ref HEAD 2>/dev/null || true)"
fi
if [[ -z "${branch}" || "${branch}" == "HEAD" ]]; then
    branch="local"
fi
branch="$(printf '%s' "${branch}" | tr -c 'A-Za-z0-9._-' '-')"

build_dir="$(cd "${repo_root}" && mkdir -p "${build_dir}" && cd "${build_dir}" && pwd)"
output_dir="$(cd "${repo_root}" && mkdir -p "${output_dir}" && cd "${output_dir}" && pwd)"
tools_dir="$(mkdir -p "${tools_dir}" && cd "${tools_dir}" && pwd)"

linuxdeploy="${LINUXDEPLOY:-${tools_dir}/linuxdeploy-${arch}.AppImage}"
linuxdeploy_url="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${arch}.AppImage"

client_appdir="${output_dir}/AppDir-client"
server_appdir="${output_dir}/AppDir-server"
client_name="multiplayer-sandbox-game-client-${branch}-${arch}"
server_name="multiplayer-sandbox-game-server-${branch}-${arch}"

download_linuxdeploy() {
    if [[ -x "${linuxdeploy}" ]]; then
        return
    fi

    mkdir -p "$(dirname "${linuxdeploy}")"
    if command -v curl >/dev/null 2>&1; then
        curl -L --fail --retry 3 -o "${linuxdeploy}" "${linuxdeploy_url}"
    elif command -v wget >/dev/null 2>&1; then
        wget -O "${linuxdeploy}" "${linuxdeploy_url}"
    else
        printf 'curl or wget is required to download linuxdeploy\n' >&2
        exit 1
    fi
    chmod +x "${linuxdeploy}"
}

copy_glob() {
    local pattern="$1"
    local destination="$2"
    local matches=()
    mapfile -t matches < <(compgen -G "${pattern}" || true)

    if (( ${#matches[@]} == 0 )); then
        printf 'Required file pattern did not match: %s\n' "${pattern}" >&2
        exit 1
    fi

    cp -P "${matches[@]}" "${destination}/"
}

copy_ldd_matches() {
    local elf_file="$1"
    local destination="$2"
    local regex="$3"

    ldd "${elf_file}" \
        | awk -v regex="${regex}" '$1 ~ regex && $3 ~ /^\// { print $3 }' \
        | sort -u \
        | while IFS= read -r library; do
            cp -L "${library}" "${destination}/"
        done
}

copy_runtime_libraries() {
    local appdir="$1"
    local include_sdl="$2"
    local lib_dir="${appdir}/usr/lib"

    mkdir -p "${lib_dir}"
    copy_glob "${build_dir}/bin/libGameNetworkingSockets.so*" "${lib_dir}"
    copy_ldd_matches "${build_dir}/bin/libGameNetworkingSockets.so" "${lib_dir}" '^(libprotobuf[.]so|libcrypto[.]so|libz[.]so)'

    if [[ "${include_sdl}" == "yes" ]]; then
        copy_glob "${build_dir}/_deps/sdl3-build/libSDL3.so*" "${lib_dir}"
    fi
}

write_apprun() {
    local appdir="$1"
    local executable="$2"

    cat > "${appdir}/AppRun" <<EOF
#!/usr/bin/env bash
set -euo pipefail
appdir="\$(cd "\$(dirname "\${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="\${appdir}/usr/lib:\${LD_LIBRARY_PATH:-}"
cd "\${appdir}/usr/bin"
exec "\${appdir}/usr/bin/${executable}" "\$@"
EOF
    chmod +x "${appdir}/AppRun"
}

remove_graphics_libraries() {
    local appdir="$1"
    local lib_dir="${appdir}/usr/lib"

    rm -f \
        "${lib_dir}"/libOpenGL.so* \
        "${lib_dir}"/libGL.so* \
        "${lib_dir}"/libGLX.so* \
        "${lib_dir}"/libGLdispatch.so* \
        "${lib_dir}"/libEGL.so* \
        "${lib_dir}"/libgbm.so* \
        "${lib_dir}"/libdrm.so* || true
}

run_linuxdeploy() {
    local appdir="$1"
    local desktop_file="$2"
    local icon_file="$3"

    APPIMAGE_EXTRACT_AND_RUN=1 "${linuxdeploy}" \
        --appdir "${appdir}" \
        --desktop-file "${desktop_file}" \
        --icon-file "${icon_file}"
}

build_appimage() {
    local appdir="$1"
    local output_name="$2"

    remove_graphics_libraries "${appdir}"
    printf '\nBundled libraries for %s:\n' "${output_name}"
    find "${appdir}/usr/lib" -maxdepth 1 \( -type f -o -type l \) | sort

    (
        cd "${output_dir}"
        ARCH="${arch}" APPIMAGE_EXTRACT_AND_RUN=1 "${linuxdeploy}" \
            --appdir "${appdir}" \
            --output appimage
    )

    local generated
    generated="$(find "${output_dir}" -maxdepth 1 \
        -name '*.AppImage' \
        ! -name "${client_name}.AppImage" \
        ! -name "${server_name}.AppImage" \
        -type f \
        -printf '%p\n' | sort | tail -n 1)"
    if [[ -z "${generated}" || ! -f "${generated}" ]]; then
        printf 'linuxdeploy did not produce an AppImage for %s\n' "${output_name}" >&2
        exit 1
    fi

    mv "${generated}" "${output_dir}/${output_name}.AppImage"
    chmod +x "${output_dir}/${output_name}.AppImage"
}

download_linuxdeploy
rm -rf "${client_appdir}" "${server_appdir}"
rm -f "${output_dir}"/*.AppImage

cmake --install "${build_dir}" --prefix "${client_appdir}/usr" --component client
cmake --install "${build_dir}" --prefix "${server_appdir}/usr" --component server

copy_runtime_libraries "${client_appdir}" yes
copy_runtime_libraries "${server_appdir}" no

write_apprun "${client_appdir}" game_client
write_apprun "${server_appdir}" game_server

run_linuxdeploy "${client_appdir}" "${repo_root}/packaging/linux/game_client.desktop" "${repo_root}/packaging/linux/game_client.svg"
run_linuxdeploy "${server_appdir}" "${repo_root}/packaging/linux/game_server.desktop" "${repo_root}/packaging/linux/game_server.svg"

build_appimage "${client_appdir}" "${client_name}"
build_appimage "${server_appdir}" "${server_name}"

printf '\nCreated AppImages:\n'
find "${output_dir}" -maxdepth 1 -name '*.AppImage' -type f -printf '%f\n' | sort
