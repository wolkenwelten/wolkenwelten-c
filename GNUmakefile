CC                   := cc
OPTIMIZATION         := -O2 -fno-lto
RELEASE_OPTIMIZATION := -O3 -flto
CINCLUDES            :=
LIBS                 := -lm
WARNINGS             := -Wall -Werror -Wextra -Wfloat-equal -Wshadow -Wcast-align
CFLAGS               := -g
CSTD                 := -std=c99

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
