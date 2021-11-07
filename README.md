# WolkenWelten

## Mixing the best parts of Minecraft, Quake ]I[ and Emacs.
WolkenWelten is meant to be the ultimate **voxel sandbox**, letting not only
players choose their own adventures in a big procedural world but giving
the ability to change the rules of said world, either by making use
of the **custom Lisp runtime**, or creating patches to the underlying C code.

While there is a default gamemode that makes players fend for themselves
on an archipelago within the clouds, this is just the beginning.
Most gameplay parts are currently rewritten to make the client as generic
as possible, enabling completely different games by just connecting to a
server.

[Twitch streams](https://twitch.tv/melchizedek6809) focussing on the development of this game happen every Saturday
and Sunday, as well as on Monday and Tuesday where we dive deep into the internals of the Nujel runtime, trying to turn it into something resembling a proper language.

This project puts a lot of effort into supporting all kinds of devices, nut just high-end gaming PCs, because of this the **Raspberry PI 4/(00)** is a viable system to play this game on, the **WASM** version should also run on many **mobile devices**, although a Gamepad of some sort is probably needed due to the touch controls being
absolutely terrible at the moment.

![screenshot](https://wolkenwelten.net/img/1.jpg)

## Some Bullet Points
* Mostly written in C99
* Fits on a **floppy disk**
* Runs on a **Raspberry PI 4**
* Play in **Multiplayer** with up to 31 people!
* Contains a custom LISP interpreter/dialect, _Nujel_
* Reprogram item behaviour, while the game is running
* Use a **grappling hook** to swing around
* Nice fire system, beware of forest fires
* Can even [run in your browser](https://wolkenwelten.net/releases/wasm/index.html?savegame=Test), using **Emscripten/WASM**

# Releases / Screenshots
The newest binary releases are available on the [projects website](https://wolkenwelten.net),
along with some more writings about the game, as well as a couple of screenshots.

## Contact
If you run into some bugs, have gameplay questions or want to talk about your
favorite *nix, join us over on [Discord](https://discord.gg/7rhnYH2), or preferrably on
[Matrix](https://matrix.to/#/!RKZztYPGhtlgALDvMS:matrix.org?via=matrix.org).

## Contributing
As a Free Software Project any form of help you can give would be highly
appreciated, be it testing the game, drawing nice artwork, coding that cool
feature or telling your friends about this game. You have my gratitude in advance :)

# Development Requirements

## Windows
On Windows you need a working installation of [MSYS2](https://www.msys2.org/),
and then install the following packages from within MSYS2:
```shell
pacman -Sy base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-ffmpeg mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_mixer
```

## MacOS
On Macintosh you need the XCode command line tools, the SDL2 and SDL2_mixer
development Frameworks installed. Additionally you need `ffmpeg`,
which is probably best installed using Homebrew or MacPorts.

### Arch Linux
```shell
pacman -S base-devel clang ffmpeg sdl2 sdl2_mixer
```

### Debian / Ubuntu / Raspberry PI OS
```shell
apt install build-essential clang ffmpeg libsdl2-dev libsdl2-mixer-dev musl-dev musl-tools libvorbis-dev
```

### OpenBSD
```shell
pkg_add clang gas gmake bash sdl2 sdl2-mixer glew ffmpeg
```
You can then build the game using `gmake`

### NetBSD
```shell
pkgin in clang gas gmake bash SDL2 SDL2_mixer glew ffmpeg
```
You can then build the game using `gmake`

# Build System
Now that you have all the packages installed you can just type `make` within
the repos folder to create a development binary, I highly recommend adding
something like `-j8` so it does not take quite as long to compile. After that
you can use `make clean` to remove every executable/intermedia file you just
created. There are many more convenience targets I added which you can take a
look at in the client/common/server Makefiles, most should be explained by the
command they execute.

To test that the nujel interpreter is working correctly just execute `make test`
to run the automated testroutines for the nujel interpreter.
