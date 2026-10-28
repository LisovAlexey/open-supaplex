#!/bin/bash
# Builds OpenSupaplex as an installable PS5 app folder (eboot.bin + sce_sys/param.json)
# that ShadowMountPlus can register. The packaging is done by
# ps5-native-app-boilerplate, which turns the game into a fake-signed native app.

set -e

BOILERPLATE_URL=https://github.com/drakmor/ps5-native-app-boilerplate.git
BOILERPLATE_COMMIT=470695e0c557f99ff2df0e36e4df713c5e636526

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(dirname "${SCRIPT_DIR}")"
WORK_DIR="${SCRIPT_DIR}/build/boilerplate"
TITLE_ID=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["titleId"])' "${SCRIPT_DIR}/param.json")

# SDL2_mixer with MOD/XM support, shared with the payload build
"${REPO_DIR}/ps5/build-sdl2-mixer.sh"
MIXER_DIR="${REPO_DIR}/ps5/build/deps"

# The checkout may belong to another user when built inside a container
git config --global --add safe.directory "${WORK_DIR}" 2>/dev/null || true

if [ ! -d "${WORK_DIR}/.git" ]; then
    git clone -q "${BOILERPLATE_URL}" "${WORK_DIR}"
fi
git -C "${WORK_DIR}" fetch -q origin "${BOILERPLATE_COMMIT}" || true
git -C "${WORK_DIR}" checkout -q --force "${BOILERPLATE_COMMIT}"
# Fixes the RELRO segment file offset when .data.rel.ro precedes .got (as in
# SDL2); without it the PS5 refuses to start the app
git -C "${WORK_DIR}" apply "${SCRIPT_DIR}/boilerplate-relro-offset.patch"

# Replace the boilerplate's sample app with OpenSupaplex
cd "${WORK_DIR}"
rm -rf src assets vendor sce_sys/pic0.dds sce_sys/pic1.dds sce_sys/snd0.at9
mkdir -p src/sdl2 src/sdl_common src/lib/ini src/null assets vendor/lib
cp "${REPO_DIR}"/src/*.c "${REPO_DIR}"/src/*.h src/
cp "${REPO_DIR}"/src/sdl2/*.c src/sdl2/
cp "${REPO_DIR}"/src/sdl_common/*.c src/sdl_common/
cp "${REPO_DIR}"/src/lib/ini/ini.[ch] src/lib/ini/
cp "${REPO_DIR}"/src/null/virtualKeyboard.c src/null/
cp "${SCRIPT_DIR}/ps5AppShims.c" src/
cp -R "${REPO_DIR}"/resources/* assets/
cp -R "${MIXER_DIR}/include" vendor/
cp "${MIXER_DIR}/lib/libSDL2_mixer.a" vendor/lib/
make pacbrew
PACBREW_LIB_DIR=$(dirname "$(find .deps/pacbrew -path '*/user/homebrew/lib/libSDL2.a' -print -quit)")
# Point SDL2 at our shims for functions native apps can't import (see ps5AppShims.c)
REDEFINE_SYMS=()
for sym in strcasestr sceKeyboardInit sceKeyboardOpen sceKeyboardReadState sceKeyboardClose \
           sceImeDialogInit sceImeDialogGetStatus sceImeDialogGetResult sceImeDialogTerm; do
    REDEFINE_SYMS+=(--redefine-sym "${sym}=osp_${sym}")
done
llvm-objcopy-18 "${REDEFINE_SYMS[@]}" "${PACBREW_LIB_DIR}/libSDL2.a" vendor/lib/libSDL2.a

# Fail early if the app would need a system module that games can't load
BLOCKED_MODULES="libkernel_sys libkernel_web libSceKeyboard libSceImeDialog"

cp "${SCRIPT_DIR}/param.json" sce_sys/param.json
cp "${REPO_DIR}/ps5/assets/sce_sys/icon0.png" sce_sys/icon0.png

# PacBrew archives are listed by hand: their pkg-config files add -l flags for
# system modules, which the boilerplate already links from the SDK stubs
make APP_DEFINITIONS="HAVE_SDL2 PS5_APP NDEBUG _DEFAULT_SOURCE" \
     APP_INCLUDE_PATHS="vendor/include vendor/include/SDL2" \
     APP_STATIC_ARCHIVES="vendor/lib/libSDL2_mixer.a vendor/lib/libSDL2.a" \
     PACBREW_INCLUDE_PATHS="include/SDL2" \
     PACBREW_STATIC_ARCHIVES="lib/libmodplug.a lib/libsamplerate.a lib/libiconv.a"

for module in ${BLOCKED_MODULES}; do
    if llvm-readelf-18 -d build/eboot.elf | grep -q "\[${module}\.prx\]"; then
        echo "error: eboot imports ${module}, which native apps can't load" >&2
        exit 1
    fi
done

cd "${WORK_DIR}/dist"
rm -f "${SCRIPT_DIR}/OpenSupaplex-ps5-app.zip"
zip -qr "${SCRIPT_DIR}/OpenSupaplex-ps5-app.zip" "${TITLE_ID}"
echo "App folder: ${WORK_DIR}/dist/${TITLE_ID}"
