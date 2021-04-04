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

VERSION_NAME         := $(shell ./nujel tools/tools.nuj -x "(display (infogen-version))")
#VERSION_NAME         := test
VERSION_ARCH         := $(shell uname -m)
AS_SYM               := NO_SYM=NO_SYM


include common/disable_implicit_rules.mk
include common/common.mk
include client/client.mk
include server/server.mk

ifeq ($(OS),Windows_NT)
	include platform/win/win.mk
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		include platform/macos/macos.mk
	endif
	ifeq ($(UNAME_S),Linux)
		include platform/linux/linux.mk
		ifdef MINGWCROSS
			include platform/win_cross/win_cross.mk
		endif
		ifdef OSXCROSS
			include platform/macos_cross/macos_cross.mk
		endif
	endif
	ifeq ($(UNAME_S),Haiku)
		include platform/haiku/haiku.mk
	endif
	ifeq ($(UNAME_S),DragonflyBSD)
		include platform/bsd/bsd.mk
	endif
	ifeq ($(UNAME_S),OpenBSD)
		include platform/bsd/bsd.mk
	endif
	ifeq ($(UNAME_S),NetBSD)
		include platform/bsd/bsd.mk
	endif
	ifeq ($(UNAME_S),FreeBSD)
		include platform/bsd/bsd.mk
	endif
endif

ifdef EMSDK
	include platform/wasm/wasm.mk
endif

ifeq ($(VERSION_ARCH),armv7l)
	LDFLAGS += -march=native
	ASFLAGS += -march=armv7-a -mfloat-abi=hard -mfpu=neon
	CFLAGS  += -march=armv7-a -mfloat-abi=hard -mfpu=neon
endif
