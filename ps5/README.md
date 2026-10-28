# OpenSupaplex for PlayStation 5

This port runs as a homebrew payload on a **jailbroken** PS5. It is built with
the [ps5-payload-sdk](https://github.com/ps5-payload-dev/sdk) and its
[SDL2 port](https://github.com/ps5-payload-dev/SDL).

## Installing

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

| DualSense       | In game                              | In menus             |
| --------------- | ------------------------------------ | -------------------- |
| D-pad / L-stick | Move                                 | Move cursor          |
| Square          | Action (eat/push in place, red disk) |                      |
| Cross           |                                      | Confirm              |
| Circle          | Show number of red disks             | Cancel               |
| Triangle        | Toggle game panel                    |                      |
| Options         | Pause                                | Start                |
| Touchpad click  | Exit level                           | Back                 |
| L1 / R1         | Game speed down / up                 | Previous / next level set |

The Create button is reserved by the system, so the touchpad click takes the
role of the Back/Select button used by other ports.

## Building

The build needs Docker (or an Ubuntu 24.04 machine with the packages from
`ci-prepare.sh`). From the repository root:

```sh
./ps5/ci-docker-run.sh bash -c "./ps5/ci-prepare.sh && ./ps5/ci-build.sh"
```

This produces `ps5/eboot.elf` and `ps5/OpenSupaplex-ps5.zip`.

With the SDK already installed in `/opt/ps5-payload-sdk`:

```sh
cd ps5
make package                 # build and zip
PS5_HOST=192.168.1.50 make test   # send eboot.elf to elfldr
```

The SDL2_mixer bundled with the SDK has no MOD/XM support, so
`build-sdl2-mixer.sh` rebuilds it with libmodplug for the game's music.
