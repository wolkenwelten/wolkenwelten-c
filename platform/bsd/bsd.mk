AS               := gas
ifeq (, $(shell which gas))
	AS       := as
endif

CC               := clang

CFLAGS           += -D_GNU_SOURCE

CLIENT_LIBS      := $(shell sdl2-config --libs) $(shell pkg-config --libs glew) -lm -lSDL2 -lSDL2_mixer
LIBS             := -lm
