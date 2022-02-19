ifneq (, $(shell which dash))
	SHELL   := $(shell which dash)
endif

ifeq (, $(wildcard ./common/nujel/LICENSE))
$(error Cant find Nujel, try loading submodules using "git submodule update --init --recursive")
endif

WOLKENWELTEN        := ./wolkenwelten
WOLKENWELTEN_SERVER := ./wolkenwelten-server
NUJEL               := ./common/nujel/nujel
ASSET               := ./tools/assets
ifeq ($(OS),Windows_NT)
	NUJEL               := ./common/nujel/nujel.exe
	ASSET               := ./tools/assets.exe
	WOLKENWELTEN        := ./wolkenwelten.exe
	WOLKENWELTEN_SERVER := ./wolkenwelten-server.exe
endif

AS                   := as
AS_SYM               := NO_SYM=NO_SYM
ASFLAGS              :=

ISPC                 := ispc
ISPC_OPTIMIZATION    := -O3
ISPC_TARGET          := --target=sse4-i8x16

CC                   := cc
CFLAGS               := -g -D_GNU_SOURCE
CSTD                 := -std=c11
OPTIMIZATION         := -O2 -fno-lto -ffast-math -freciprocal-math
WARNINGS             := -Wall -Werror -Wextra -Wshadow -Wcast-align -Wno-missing-braces

LIBS                 :=

RELEASE_OPTIMIZATION := -O3 -flto -ffast-math -freciprocal-math
VERSION_ARCH         := $(shell uname -m)

ifneq (, $(shell which $(NUJEL)))
	VERSION_NAME := $(shell $(NUJEL) tools/tools.nuj -x "[display [infogen-version]]")
endif

all: $(WOLKENWELTEN) $(WOLKENWELTEN_SERVER) $(NUJEL)
.PHONY: all release .deps

 $(NUJEL):
	@$(MAKE) -C common/nujel

include common/disable_implicit_rules.mk
include common/ansi_colors.mk
include common/common.mk
include client/client.mk
include server/server.mk

ifneq ($(MAKECMDGOALS),clean)
-include $(CLIENT_DEPS)
-include $(COMMON_DEPS)
-include $(SERVER_DEPS)
endif

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
	ifeq ($(UNAME_S),DragonFly)
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
