# WolkenWelten

![Have a schway Logo](https://wolkenwelten.net/img/logo.png)

## Imagine a mix between Minecraft, Quake ]I[ and Emacs.
This game is meant to be a cooperative multiplayer survival game, using voxels
to enable a fully modifiable world. Although it is turning into some sort of
LISP-Powered Voxel Runtime, we will see where that goes.
There is no definitive plan for this game, mostly ideas to experiment with until
something fun surfaces. You can read about some ideas in this repository, though
some have only been talked about during the Weekly [Twitch](https://twitch.tv/melchizedek6809)
dev streams.

![Screenshot 1](https://wolkenwelten.net/img/1.jpg)

### Some Bullet Points
* Mostly written in C99
* Fits on a **floppy disk**
* Runs at **~60FPS** on a **Raspberry PI 4**
* Play in **Multiplayer** with up to 31 people!
* Contains a custom LISP interpreter/dialect, _nujel_
* Reprogram item behaviour, while the game is running
* Use a **grappling hook** to swing around
* Nice fire system, beware of forest fires
* Can even run in your browser, using **Emscripten/WASM**

![Screenshot 2](https://wolkenwelten.net/img/2.jpg)

## Contact
If you run into some bugs, have gameplay questions or want to talk about your
favorite *nix, join us over on [Discord](https://discord.gg/7rhnYH2), or preferrably on
[Matrix](https://matrix.to/#/!RKZztYPGhtlgALDvMS:matrix.org?via=matrix.org).

![Screenshot 4](https://wolkenwelten.net/img/4.jpg)

## Contributing
As a Free Software Project any form of help you can give would be highly
appreciated, be it testing the game, drawing nice artwork, coding that cool
feature or telling your friends about this game. You have my gratitude in advance :)

## Release Builds
The newest binary releases are available over at this [projects website](https://wolkenwelten.net),
along with some more writings about the game.

![Screenshot 5](https://wolkenwelten.net/img/5.jpg)

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
