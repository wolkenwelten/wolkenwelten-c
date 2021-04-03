AS               := gas
ifeq (, $(shell which gas))
	AS       := as
endif

CC               := clang
LD               := clang

CFLAGS           += -D_GNU_SOURCE
LDFLAGS          += -D_GNU_SOURCE

CLIENT_CINCLUDES := $(shell sdl2-config --cflags) $(shell pkg-config --cflags glew)
CLIENT_LIBS      := $(shell sdl2-config --libs) $(shell pkg-config --libs glew) -lm -lSDL2 -lSDL2_mixer
SERVER_LIBS      := -lm

.PHONY: all
all: wolkenwelten wolkenwelten-server