NUJEL_SRCS   := $(shell find nujel-standalone -type f -name '*.c') $(shell find common/src/nujel -type f -name '*.c') common/src/misc/vec.c
NUJEL_HDRS   := $(shell find nujel-standalone -type f -name '*.h') $(shell find common/src/nujel -type f -name '*.h') common/src/misc/vec.h
NUJEL_OBJS   := $(NUJEL_SRCS:.c=.o)
NUJEL_DEPS   := ${NUJEL_SRCS:.c=.d}
SAOLIB_NUJS  := $(shell find nujel-standalone/lib -type f -name '*.nuj')
STDLIB_NUJS  := $(shell find common/src/nujel/stdlib -type f -name '*.nuj')

$(NUJEL): $(NUJEL_SRCS) nujel-standalone/tmp/assets.c
	$(CC) -D_GNU_SOURCE $(LIBS) $(CINCLUDES) $(WARNINGS) $(CSTD) $(OPTIMIZATION) $^ -o $(NUJEL) && ./$(NUJEL) -x "(quit (test-run))"

-include $(NUJEL_DEPS)

nujel-standalone/tmp/saolib.nuj: $(SAOLIB_NUJS)
	@mkdir -p nujel-standalone/tmp/
	cat $^ > $@
nujel-standalone/tmp/assets.c: nujel-standalone/tmp/saolib.nuj common/src/tmp/stdlib.nuj $(ASSET)
	@mkdir -p nujel-standalone/tmp/
	./$(ASSET) nujel-standalone/tmp/assets nujel-standalone/tmp/saolib.nuj common/src/tmp/stdlib.nuj
nujel-standalone/tmp/assets.h: nujel-standalone/tmp/assets.c
	@true

.PHONY: test
test: nujel
	./$(NUJEL) -x "(quit (test-run))"

.PHONY: runn
runn: nujel
	gdb $(NUJEL) -ex "r"
