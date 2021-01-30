Good morning,

this Project is still in early Development so I highly recommend against writing
patches/mods for it, as your work will probably go to waste due to some
restructuring, although the codebase is getting more stable :)

You can watch me develop this game over at [Twitch](https://twitch.tv/melchizedek6809)
and if you run into some bugs, have gameplay questions or want to talk about your
favorite *nix, join us on [Discord](https://discord.gg/7rhnYH2) or preferrably on
[Matrix](https://matrix.to/#/!RKZztYPGhtlgALDvMS:matrix.org?via=matrix.org).

The newest releases are available over at this [projects website](https://wolkenwelten.net)
, along with some descriptive text

# Software Requirments

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
