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

.PHONY: release
.PHONY: .deps

%.o: %.c
	$(CC) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(CFLAGS) $(CINCLUDES) -c $< -o $@

.PHONY: clean
clean:
	rm -f gmon.out client/make.deps client/tools/assets client/tools/objparser callgrind.out.* vgcore.* platform/win/wolkenwelten.res
	rm -f $(shell find common/src -type f -name '*.o')
	rm -f wolkenwelten wolkenwelten.exe $(shell find client/src -type f -name '*.o')
	rm -f wolkenwelten-server wolkenwelten-server.exe $(shell find server/src -type f -name '*.o')
	rm -rf client/src/tmp/ server/src/tmp/
	rm -rf web/releases releases client/src/tmp

.PHONY: webrelease
webrelease: release
	rsync -avhe ssh $(WEBEXCLUDE) releases wolkenwelten.net:/var/www/wolkenwelten.net/

.PHONY: website
website:
	rsync -avhe ssh web/ wolkenwelten.net:/var/www/wolkenwelten.net/

.PHONY: web
web: webrelease website

.PHONY: debug
debug: CFLAGS += -O0
debug: all

.PHONY: profile
profile: CFLAGS += -pg
profile: all

.PHONY: sanitize
sanitize: CFLAGS += -fsanitize=address
sanitize: all

.PHONY: rund
rund: all
	./wolkenwelten -soundVolume=10 -worldSeed=11 -debugInfo=1 -fullscreen

.PHONY: run
run: all
	./wolkenwelten

.PHONY: archive
archive:
	git archive --format=tar --prefix=wolkenwelten-HEAD.tar.gz/ HEAD | gzip > wolkenwelten-HEAD.tar.gz
