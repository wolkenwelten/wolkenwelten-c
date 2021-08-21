AS := as
CC := gcc

ASFLAGS += -gdwarf-2
CFLAGS  += -gdwarf-2

CLIENT_LIBS := -lnetwork -lm $(shell sdl2-config --libs) -lGL -lGLEW -lpthread -lSDL2 -lSDL2_mixer
LIBS        := -lnetwork -lm
