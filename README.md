# WolkenWelten

![Screenshot 1](https://wolkenwelten.net/img/1.jpg)

## Imagine a mix between Minecraft, Quake ]I[ and Emacs.
Apart from that there is no clear plan for this game, just a bunch of ideas that
hopefully will turn out to be fun. Some of these are stored here in this repo,
others have been talked about on [Twitch](https://twitch.tv/melchizedek6809)
during dev streams.

* mostly written in C99
* fits on a floppy disk
* runs at ~60FPS on a Raspberry PI
* multiplayer!
* contains a custom LISP interpreter
* use a grappling hook to swing around
* nice fire simulation
* can even run in your browser

![Screenshot 2](https://wolkenwelten.net/img/2.jpg)

## Contact
If you run into some bugs, have gameplay questions or want to talk about your
favorite *nix, join us over on [Discord](https://discord.gg/7rhnYH2), or preferrably on
[Matrix](https://matrix.to/#/!RKZztYPGhtlgALDvMS:matrix.org?via=matrix.org), there
is also a #WolkenWelten IRC channel on Freenode.

![Screenshot 4](https://wolkenwelten.net/img/4.jpg)

## Contributing
As a Free Software Project any form of help you can give would be highly
appreciated, be it testing the game, drawing nice artwork, coding that cool
feature or telling your friends about this game. You have my gratitude :)

## Release Builds
The newest binary releases are available over at this [projects website](https://wolkenwelten.net),
along with some more writings about the game.

![Screenshot 3](https://wolkenwelten.net/img/3.jpg)
![Screenshot 5](https://wolkenwelten.net/img/5.jpg)
![Screenshot 6](https://wolkenwelten.net/img/6.jpg)

# Development Requirements

## Windows
On Windows you need a working installation of msys2 and the mingw-w64 64-bit
toolchain installed, apart from that you need `SDL2`,`SDL2_mixer` and `ffmpeg`,
which you can install using pacman, or build slimmer versions using the
PKGBUILDS in `platform/win`.

## MacOS
On Windows you need the XCode command line tools, the SDL2 and SDL2_mixer
development Frameworks installed as well as the 10.10 SDK. Additionally you need
`ffmpeg` available, which is probably best installed using Homebrew.

## Linux/BSD/Haiku
You need your distributions development tools meta package ( `build-essentials`
on Ubuntu/Debian, `base-devel` on Arch), `clang`,`SDL2`,`SDL2_mixer` and
`ffmpeg`, everything of course with their dev packages for the header files.

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
