CC     := cc
CFLAGS := -fPIC
COPT   := -O2 -ffast-math
CSTD   := -std=c11
CWARN  := -Wextra -Werror -Wall -Wcast-align -pedantic
LIBS   := -lm

SRCS   := $(shell find ./ -type f -name '*.c') $(shell find ../src/nujel/ -type f -name '*.c')

.PHONY: all
all: nujel

nujel: $(SRCS)
	$(CC) $(LIBS) $(COPT) $(CWARN) $(CSTD) $(CFLAGS) -g $(SRCS) -o $@

.PHONY: run
run: nujel
	./nujel

.PHONY: clean
clean:
	rm -f nujel

.PHONY: test
test: nujel
	./test

.PHONY: runl
runl: test

.PHONY: musl
musl: $(SRCS)
	musl-gcc $(LIBS) $(COPT) $(CWARN) $(CSTD) $(CFLAGS) $(SRCS) -o nujel
