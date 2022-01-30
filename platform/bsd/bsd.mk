AS               := gas
ifeq (, $(shell which gas))
	AS       := as
endif

CC       := gcc
ifneq (, $(shell which clang))
	CC       := clang
endif

CFLAGS           += -D_GNU_SOURCE

CLIENT_LIBS      := -lm $(shell pkg-config --libs sdl2 SDL2_mixer gl)
LIBS             := -lm
