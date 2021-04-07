CC                   := cc
OPTIMIZATION         := -O2
CINCLUDES            :=
LIBS                 := -lm
WARNINGS             := -Wall -Werror -Wextra -Wshadow -Wcast-align -Wno-missing-braces
CFLAGS               := -g
CSTD                 := -std=c99
VERSION_ARCH         := $(shell uname -m)
NUJEL_SRCS           := $(shell find ./ -type f -name '*.c') $(shell find ../common/src/nujel -type f -name '*.c') ../common/src/misc/vec.c
NUJEL_HDRS           := $(shell find ./ -type f -name '*.h') $(shell find ../common/src/nujel -type f -name '*.h') ../common/src/misc/vec.h
SAOLIB_NUJS          := $(shell find ./lib -type f -name '*.nuj')
STDLIB_NUJS          := $(shell find ../common/src/nujel/stdlib -type f -name '*.nuj')

ifeq ($(VERSION_ARCH),armv7l)
	CFLAGS  += -march=armv7-a -mfloat-abi=hard -mfpu=neon
endif

ifeq ($(OS),Windows_NT)
	NUJEL := ../nujel.exe
	ASSET := ../tools/assets.exe
else
	NUJEL := ../nujel
	ASSET := ../tools/assets
endif

$(NUJEL): $(NUJEL_SRCS) $(NUJEL_HDRS) tmp/assets.c tmp/assets.h
	$(CC) -DNUJEL_STANDALONE $(CFLAGS) $(LIBS) $(CINCLUDES) $(WARNINGS) $(CSTD) $(OPTIMIZATION) $(NUJEL_SRCS) tmp/assets.c -o $(NUJEL)

tmp/saolib.nuj: $(SAOLIB_NUJS)
	mkdir -p tmp/
	cat $^ > $@
tmp/stdlib.nuj: $(STDLIB_NUJS)
	mkdir -p tmp/
	cat $^ > $@
../tools/assets: ../tools/assets.c
	$(CC) $(CFLAGS) $(LIBS) $(CINCLUDES) $(WARNINGS) $(CSTD) $(OPTIMIZATION) ../tools/assets.c -o $(ASSET)
tmp/assets.c: tmp/saolib.nuj tmp/stdlib.nuj ../tools/assets
	mkdir -p tmp/
	../tools/assets tmp/assets tmp/saolib.nuj tmp/stdlib.nuj
tmp/assets.g: tmp/assets.c
	@true

.PHONY: clean
clean:
	rm -f $(NUJEL)
