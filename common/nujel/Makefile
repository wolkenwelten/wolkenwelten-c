CC     :=cc
CFLAGS :=
COPT   :=-Os
CSTD   := -std=c11
CWARN  := -Wextra -Werror -Wall -Wcast-align -pedantic

SRCS   := $(shell find ./ -type f -name '*.c')
OBJS   := ${SRCS:.c=.o}

.PHONY: all
all: nujel

%.o: %.c
	$(CC) $(COPT) $(CWARN) $(CSTD) $(CFLAGS) -g -c $< -o $@

nujel: $(OBJS)
	$(CC) $(COPT) $(CWARN) $(CSTD) $(CFLAGS) -g $(OBJS) -o $@

.PHONY: run
run: nujel
	./nujel

.PHONY: clean
clean:
	rm -f nujel *.o

.PHONY: test
test: nujel
	./test

.PHONY: musl
musl: $(SRCS)
	$(CC) $(COPT) $(CWARN) $(CSTD) $(CFLAGS) $(SRCS) -o nujel
