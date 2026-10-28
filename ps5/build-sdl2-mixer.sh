#!/bin/bash
# The SDL2_mixer that ships with the PS5 payload SDK has no MOD/XM support,
# which OpenSupaplex needs for its music. Rebuild it with libmodplug (which
# the SDK already provides) and install it next to this script.

set -e

if [ -z "${PS5_PAYLOAD_SDK}" ]; then
    export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
fi

SDL2_MIXER_VERSION=2.8.0
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORK_DIR="${SCRIPT_DIR}/build/sdl2_mixer"
INSTALL_DIR="${SCRIPT_DIR}/build/deps"

if [ -f "${INSTALL_DIR}/lib/libSDL2_mixer.a" ]; then
    echo "SDL2_mixer already built"
    exit 0
fi

source "${PS5_PAYLOAD_SDK}/toolchain/prospero.sh"

mkdir -p "${WORK_DIR}"
cd "${WORK_DIR}"

wget -q -O SDL2_mixer.tar.gz \
    "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL2_MIXER_VERSION}/SDL2_mixer-${SDL2_MIXER_VERSION}.tar.gz"
tar xf SDL2_mixer.tar.gz
cd "SDL2_mixer-${SDL2_MIXER_VERSION}"

# Install into our own prefix instead of the SDK sysroot
unset DESTDIR

./configure --prefix="${INSTALL_DIR}" --host=x86_64-pc-freebsd \
            --enable-static --disable-shared \
            --enable-music-mod --enable-music-mod-modplug \
            --disable-music-mod-modplug-shared --disable-music-mod-xmp \
            --disable-music-midi-fluidsynth --disable-music-opus \
            --disable-music-flac-libflac --disable-music-mp3-mpg123 \
            --disable-music-wavpack \
            MODPLUG_CFLAGS="-I${PS5_SYSROOT}${PS5_HBROOT}/include/libmodplug -DMODPLUG_STATIC" \
            MODPLUG_LIBS="-L${PS5_SYSROOT}${PS5_HBROOT}/lib -lmodplug"
${MAKE} -j"$(nproc)"
${MAKE} install
