# OpenSupaplex for PlayStation 5

There are two builds for **jailbroken** PS5s, both using the
[ps5-payload-sdk](https://github.com/ps5-payload-dev/sdk) and its
[SDL2 port](https://github.com/ps5-payload-dev/SDL):

- `OpenSupaplex-ps5-app.zip`: an app folder (`PPSA90100`) with `eboot.bin` and
  `sce_sys/param.json`, for [ShadowMountPlus](https://github.com/drakmor/ShadowMountPlus).
  It shows up on the home screen like a regular game.
- `OpenSupaplex-ps5.zip`: an ELF payload for the websrv homebrew launcher.

## Installing the app (ShadowMountPlus)

1. Extract `OpenSupaplex-ps5-app.zip` and copy the `PPSA90100` folder to
   `/data/homebrew/PPSA90100` or to `homebrew/PPSA90100` on a USB drive.
2. Wait for ShadowMountPlus to pick it up (a notification is shown), then start
   OpenSupaplex from the home screen.

The game data is read from `/app0/assets/`; saves and settings are stored in
`/download0/`. The game writes a log to `/data/opensupaplex.log`, which can be
fetched over FTP when something goes wrong.

Quitting from the game menu ends with a crash report from the system (native
apps can't call `exit()` cleanly yet); close the game with the PS button
instead. Launch it while no other homebrew is running: starting it during a
game switch can leave ShadowMountPlus stuck until the console is restarted.

Notes on the native app build (see `ps5-app/`):

- Native apps can't load `libkernel_sys`, `libSceKeyboard` or
  `libSceImeDialog`, so SDL's keyboard support is stubbed out and SDL_mixer is
  built without external music commands. `build.sh` fails if any of these
  modules sneaks back in.
- The libc heap of a native app is only ~13 MB, so SDL allocates large blocks
  with `mmap` from the flexible memory budget (`initializeAppMemory`).
- `boilerplate-relro-offset.patch` fixes the boilerplate's ELF converter for
  binaries that have `.data.rel.ro` before `.got` (SDL2 does).

## Installing the payload (websrv)

1. Jailbreak your PS5 and start [elfldr](https://github.com/ps5-payload-dev/elfldr)
   and [websrv](https://github.com/ps5-payload-dev/websrv) (or a jailbreak
   bundle that already includes them, such as etaHEN-based ones).
2. Extract `OpenSupaplex-ps5.zip` and copy the `OpenSupaplex` folder to one of:
   - `/data/homebrew/OpenSupaplex` (internal storage, e.g. over FTP)
   - `/mnt/usb0/homebrew/OpenSupaplex` (root of a USB drive: `homebrew/OpenSupaplex`)
   - `/mnt/ext0/homebrew/OpenSupaplex` (extended storage)
3. Open the websrv homebrew launcher (the Homebrew Launcher PKG, or
   `http://<ps5-ip>:8080` in a browser) and start **OpenSupaplex**.

The game can also be started by sending `eboot.elf` straight to elfldr
(port 9021). In that case it looks for its data in the folders listed above.

Saves, settings and the hall of fame are written to the game folder.

## Controls

In a level:

| DualSense                 | Action                                               |
| ------------------------- | ---------------------------------------------------- |
| D-pad / left stick        | Move Murphy                                          |
| Square + direction        | Eat/take the adjacent tile without moving, use a terminal |
| Square (hold, no direction) | Drop a red disk                                    |
| Circle                    | Show the number of red disks                         |
| Triangle                  | Toggle the bottom panel                              |
| Options                   | Pause                                                |
| Touchpad click            | Exit the level                                       |
| L1 / R1                   | Decrease / increase game speed                       |

In menus:

| DualSense          | Action                         |
| ------------------ | ------------------------------ |
| D-pad / left stick | Move the cursor                |
| Cross              | Confirm                        |
| Circle             | Cancel / back                  |
| Options            | Start the level                |
| L1 / R1            | Previous / next level set      |
| Touchpad click     | Advanced options menu          |

The Create button is reserved by the system, so the touchpad click takes the
role of the Back/Select button used by other ports.

## Building

The build needs Docker (or an Ubuntu 24.04 machine with the packages from
`ci-prepare.sh`). From the repository root:

```sh
./ps5/ci-docker-run.sh bash -c "./ps5/ci-prepare.sh && ./ps5/ci-build.sh"
```

This produces `ps5/eboot.elf` and `ps5/OpenSupaplex-ps5.zip`.

The app version is built the same way with `ps5-app` instead of `ps5`:

```sh
./ps5/ci-docker-run.sh bash -c "./ps5-app/ci-prepare.sh && ./ps5-app/ci-build.sh"
```

It produces `ps5-app/OpenSupaplex-ps5-app.zip`, packaged by
[ps5-native-app-boilerplate](https://github.com/drakmor/ps5-native-app-boilerplate)
(see `ps5-app/build.sh`).

With the SDK already installed in `/opt/ps5-payload-sdk`:

```sh
cd ps5
make package                 # build and zip
PS5_HOST=192.168.1.50 make test   # send eboot.elf to elfldr
```

The SDL2_mixer bundled with the SDK has no MOD/XM support, so
`build-sdl2-mixer.sh` rebuilds it with libmodplug for the game's music.
