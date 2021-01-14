CC     := cc
CFLAGS := -fPIC
COPT   := -O2 -ffast-math
CSTD   := -std=c11
CWARN  := -Wextra -Werror -Wall -Wcast-align -pedantic
LIBS   := -lm

SRCS   := $(shell find ./ -type f -name '*.c') $(shell find ../src/nujel/ -type f -name '*.c')
OBJS   := ${SRCS:.c=.o}

.PHONY: all
all: nujel

%.o: %.c
	$(CC) $(COPT) $(CWARN) $(CSTD) $(CFLAGS) -g -c $< -o $@

main.o: main.c
main.o: ../src/common.h
main.o: ../src/nujel/arithmetic.h
main.o: ../src/nujel/nujel.h
main.o: ../src/nujel/string.h

nujel: $(OBJS)
	$(CC) $(LIBS) $(COPT) $(CWARN) $(CSTD) $(CFLAGS) -g $(OBJS) -o $@

.PHONY: run
run: nujel
	./nujel

.PHONY: clean
clean:
	rm -f nujel *.o

.PHONY: test
test: nujel
	./test

.PHONY: runl
runl: test

.PHONY: musl
musl: $(SRCS)
	$(CC) $(COPT) $(CWARN) $(CSTD) $(CFLAGS) $(SRCS) -o nujel
