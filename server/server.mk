SERVER_LIBS      := -lm

SERVER_HDRS      := $(shell find server/src -type f -name '*.h') $(COMMON_HDRS)
SERVER_SRCS_EXCL := $(shell find server/src -type f -name '*.c')
SERVER_SRCS      := $(SERVER_SRCS_EXCL) $(COMMON_SRCS)
SERVER_TMP_SRCS  := common/src/tmp/assets.c common/src/tmp/cto.c server/src/tmp/assets.c server/src/tmp/objs.c server/src/tmp/sfx.c
SERVER_TMP_OBJS  := ${SERVER_TMP_SRCS:.c=.o}
SERVER_OBJS_EXCL := ${SERVER_SRCS_EXCL:.c=.o}
SERVER_OBJS      := $(SERVER_OBJS_EXCL) $(SERVER_TMP_OBJS) ${COMMON_OBJS}
SERVER_DEPS      := ${SERVER_SRCS:.c=.d}

SERVER_NUJ       := $(shell find server/src/nujel -type f -name '*.nuj' | sort)
SERVER_ASSETS    := server/src/tmp/server.nuj

$(SERVER_OBJS_EXCL): | common/src/tmp/cto.c
$(SERVER_OBJS): | server/src/tmp/assets.h
$(SERVER_OBJS): | server/src/tmp/objs.h
$(SERVER_OBJS): | server/src/tmp/sfx.h

$(WOLKENWELTEN_SERVER): $(SERVER_OBJS) $(ASM_OBJS) ${SERVER_TMP_OBJS} common/nujel/nujel.a common/nujel/tmp/stdlib.o
	$(CC) -D_GNU_SOURCE $^ -g -o $@ $(OPTIMIZATION) $(CFLAGS) $(SERVER_CFLAGS) $(CINCLUDES) $(SERVER_CINCLUDES) $(SERVER_LIBS) $(CSTD)
	@echo "$(ANSI_BG_GREEN)" "[LD] " "$(ANSI_RESET)" $@

server/src/tmp/server.nuj: $(SERVER_NUJ)
	@mkdir -p server/src/tmp
	@cat $(SERVER_NUJ) > $@
	@echo "$(ANSI_GREY)" "[CAT]" "$(ANSI_RESET)" $@

server/src/tmp/sfx.c: $(SFX_ASSETS)
server/src/tmp/sfx.c: server/tools/sfxgen
	@mkdir -p server/src/tmp/
	@server/tools/sfxgen server/src/tmp/sfx client/sfx/
	@echo "$(ANSI_GREY)" "[SFX]" "$(ANSI_RESET)" $@
server/src/tmp/sfx.h: server/src/tmp/sfx.c
	@true

server/src/tmp/objs.c: $(MESH_ASSETS)
server/src/tmp/objs.c: server/tools/objgen
	@mkdir -p server/src/tmp/
	@server/tools/objgen server/src/tmp/objs client/mesh/
	@echo "$(ANSI_GREY)" "[OBJ]" "$(ANSI_RESET)" $@
server/src/tmp/objs.h: server/src/tmp/objs.c
	@true

server/src/tmp/assets.c: $(ASSET) $(SERVER_ASSETS)
	@mkdir -p server/src/tmp/
	@$(ASSET) server/src/tmp/assets $(SERVER_ASSETS)
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
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
	valgrind --tool=callgrind --dump-instr=yes ./wolkenwelten-server $(TEST_WORLD) -profileWG
	kcachegrind
