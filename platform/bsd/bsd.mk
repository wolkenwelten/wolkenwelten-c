AS               := gas
ifeq (, $(shell command -v gas))
	AS       := as
endif

CC       := gcc
ifneq (, $(shell command -v clang))
	CC       := clang
endif

CFLAGS           += -D_GNU_SOURCE

CLIENT_LIBS      := -lm $(shell pkg-config --libs sdl2 SDL2_mixer gl)
LIBS             := -lm
