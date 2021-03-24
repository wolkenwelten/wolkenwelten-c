AS                   := as
ASFLAGS              :=
CC                   := cc
OPTIMIZATION         := -O2 -fno-lto -ffast-math -freciprocal-math
RELEASE_OPTIMIZATION := -O3 -flto -ffast-math -freciprocal-math
CINCLUDES            :=
LIBS                 := -lm
WARNINGS             := -Wall -Werror -Wextra -Wshadow -Wcast-align
CFLAGS               := -g
CSTD                 := -std=c99

VERSION_NAME         := $(shell tools/versionname)
VERSION_ARCH         := $(shell uname -m)
AS_SYM               := NO_SYM=NO_SYM

include common/Makefile.common
include client/Makefile.client
include server/Makefile.server

ifeq ($(OS),Windows_NT)
	include platform/win/Makefile.win
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		include platform/macos/Makefile.macos
	endif
	ifeq ($(UNAME_S),Linux)
		include platform/linux/Makefile.linux
		ifdef MINGWCROSS
			include platform/win_cross/Makefile.win
		endif
		ifdef OSXCROSS
			include platform/macos_cross/Makefile.macos
		endif
	endif
	ifeq ($(UNAME_S),Haiku)
		include platform/haiku/Makefile.haiku
	endif
	ifeq ($(UNAME_S),OpenBSD)
		include platform/bsd/Makefile.bsd
	endif
	ifeq ($(UNAME_S),NetBSD)
		include platform/bsd/Makefile.bsd
	endif
	ifeq ($(UNAME_S),FreeBSD)
		include platform/bsd/Makefile.bsd
	endif
endif

ifdef EMSDK
	include platform/wasm/Makefile.wasm
endif

ifeq ($(VERSION_ARCH),armv7l)
	LDFLAGS += -march=native
	ASFLAGS += -march=armv7-a -mfloat-abi=hard -mfpu=neon
	CFLAGS  += -march=armv7-a -mfloat-abi=hard -mfpu=neon
endif
