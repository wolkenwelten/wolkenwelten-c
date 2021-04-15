SERVER_CFLAGS    :=
SERVER_CINCLUDES :=
SERVER_LIBS      :=

SERVER_HDRS      := $(shell find server/src -type f -name '*.h') $(COMMON_HDRS)
SERVER_SRCS      := $(shell find server/src -type f -name '*.c') $(COMMON_SRCS)
SERVER_OBJS      := ${SERVER_SRCS:.c=.o}
SERVER_DEPS      := ${SERVER_SRCS:.c=.d}

SFX_ASSETS       := $(shell find client/sfx -type f -name '*')
MESH_ASSETS      := $(shell find client/mesh -type f -name '*')
SERVER_NUJ       := $(shell find server/src/nujel/ -type f -name '*.nuj' | sort)
SERVER_ASSETS    := server/src/tmp/server.nuj

$(SERVER_OBJS): CFLAGS    += $(SERVER_CFLAGS)
$(SERVER_OBJS): CINCLUDES += $(SERVER_CINCLUDES)

wolkenwelten-server: CFLAGS    += $(SERVER_CFLAGS)
wolkenwelten-server: CINCLUDES += $(SERVER_CINCLUDES)
wolkenwelten-server: LIBS      += $(SERVER_LIBS)
wolkenwelten-server: $(SERVER_OBJS) $(ASM_OBJS) common/src/tmp/cto.o server/src/tmp/assets.o
	$(LD) $^ -g -o wolkenwelten-server $(OPTIMIZATION) $(LDFLAGS) $(CINCLUDES) $(LIBS)


$(SERVER_DEPS): | server/src/tmp/objs.h server/src/tmp/sfx.h
.deps: server/server.d
server/server.d: $(SERVER_DEPS)
	cat $(SERVER_DEPS) > server/server.d

ifneq ($(MAKECMDGOALS),clean)
-include server/server.d
endif

server/src/tmp/server.nuj: $(SERVER_NUJ)
	@mkdir -p server/src/tmp
	cat $(SERVER_NUJ) > $@

server/src/tmp/sfx.c: $(SFX_ASSETS)
server/src/tmp/sfx.c: server/tools/sfxgen
	@mkdir -p server/src/tmp/
	server/tools/sfxgen server/src/tmp/sfx client/sfx/
server/src/tmp/sfx.h: server/src/tmp/sfx.c
	@true

server/src/tmp/objs.c: $(MESH_ASSETS)
server/src/tmp/objs.c: server/tools/objgen
	@mkdir -p server/src/tmp/
	server/tools/objgen server/src/tmp/objs client/mesh/
server/src/tmp/objs.h: server/src/tmp/objs.c
	@true

server/src/tmp/assets.c: $(ASSET) $(SERVER_ASSETS)
	@mkdir -p server/src/tmp/
	./$(ASSET) server/src/tmp/assets $(SERVER_ASSETS)
server/src/tmp/assets.h: server/src/tmp/assets.c
	@true

server/src/network/server.o: server/src/network/server_bsd.h
server/src/network/server.o: server/src/network/server_win.h

.PHONY: run
runs: all
	./wolkenwelten-server $(TEST_WORLD)

.PHONY: run
runsv: all
	./wolkenwelten-server $(TEST_WORLD) -verbose

.PHONY: run
runsd: all
	gdb ./wolkenwelten-server -ex "r $(TEST_WORLD)"

.PHONY: run
runsp: all
	rm -rf save/ callgrind.*
	/usr/bin/time valgrind --tool=callgrind --dump-instr=yes ./wolkenwelten-server $(TEST_WORLD) -profileWG
	kcachegrind
