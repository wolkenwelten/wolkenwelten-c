ARCH             := $(shell uname -m)
NUJ_WWLIB        := $(shell find common/src/mods/ -type f -name '*.nuj' | sort)
NO_WWLIB         := $(NUJ_WWLIB:.nuj=.no)
COMMON_ASSETS    := common/src/tmp/wwlib.nuj
COMMON_HDRS      := $(shell find common/src -type f -name '*.h')
COMMON_SRCS      := $(shell find common/src -type f -name '*.c')
COMMON_OBJS      := ${COMMON_SRCS:.c=.o}
COMMON_DEPS      := ${COMMON_SRCS:.c=.d}

WEBEXCLUDE       += --exclude=releases/macos/wolkenwelten.iconset/
WEBEXCLUDE       += --exclude=releases/win/*.res

TEST_WORLD       := -worldSeed=68040 -savegame=Test

ifeq ("$(ARCH)","amd64")
ARCH             := x86_64
endif
ASM_DIR          := common/src/asm/$(ARCH)
ASM_SRCS_S       := $(shell find $(ASM_DIR) -type f -name '*.s')
ASM_SRCS_ASM     := $(shell find $(ASM_DIR) -type f -name '*.asm')
ASM_OBJS          = ${ASM_SRCS_S:.s=.o} ${ASM_SRCS_ASM:.asm=.o}

$(COMMON_OBJS): | client/src/tmp/objs.h
$(COMMON_OBJS): | client/src/tmp/sfx.h
$(COMMON_OBJS): | common/src/tmp/assets.h
$(COMMON_OBJS): | server/src/tmp/assets.h
$(COMMON_OBJS): | server/src/tmp/objs.h
$(COMMON_OBJS): | server/src/tmp/sfx.h

common/nujel/tmp/stdlib.o: $(NUJEL)
common/nujel/nujel.a: $(NUJEL)

%.o: %.asm
	@$(NASM) $(NASMFLAGS) $< -o $@
	@echo "$(ANSI_BLUE)" "[ASM]" "$(ANSI_RESET)" $@

%.o: %.s
	@$(AS) $(ASFLAGS) -c $(AS_SYM) -I $(ASM_DIR) $< -o $@
	@echo "$(ANSI_BLUE)" "[ASM]" "$(ANSI_RESET)" $@

%.o: %.c
	@$(CC) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(CFLAGS) $(CINCLUDES) $(if $(findstring client/, $<),$(CLIENT_CFLAGS) $(CLIENT_CINCLUDES),) -g -c $< -o $@ -MMD > ${<:.c=.d}
	@echo "$(ANSI_GREEN)" "[CC] " "$(ANSI_RESET)" $@

%.no: %.nuj $(NUJEL)
	@$(NUJEL) -x "[try repl/exception-handler [file/compile \"$<\"]]"
	@echo "$(ANSI_YELLOW)" "[NUJ]" "$(ANSI_RESET)" $@

common/nujel/nujel.a:
	@$(MAKE) --no-print-directory -C common/nujel nujel.a

common/src/tmp/wwlib.no: $(NO_WWLIB)
	@mkdir -p common/src/tmp
	@cat $^ > $@
	@echo "$(ANSI_GREY)" "[CAT]" "$(ANSI_RESET)" $@

common/src/tmp/wwlib.nuj: $(NUJ_WWLIB)
	@mkdir -p common/src/tmp
	@cat $^ > $@
	@echo "$(ANSI_GREY)" "[CAT]" "$(ANSI_RESET)" $@

common/src/tmp/assets.c: $(ASSET) $(COMMON_ASSETS)
	@mkdir -p common/src/tmp/
	@$(ASSET) common/src/tmp/assets $(COMMON_ASSETS)
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
common/src/tmp/assets.h: common/src/tmp/assets.c
	@true

$(ASSET): tools/assets.c
	@$(CC) $(OPTIMIZATION) $(CSTD) $(CFLAGS) $^ -o $@
	@echo "$(ANSI_BG_GREY)" "[CC] " "$(ANSI_RESET)" $@

.PHONY: clean
clean:
	@$(MAKE) --no-print-directory -C common/nujel clean
	@rm -f gmon.out client/tools/assets client/tools/objparser callgrind.out.* vgcore.* platform/win/wolkenwelten.res
	@rm -f client/sfx/*.ogg
	@rm -f $(shell find client/src common/src server/src -type f -name '*.o')
	@rm -f $(shell find client/src common/src server/src -type f -name '*.wo')
	@rm -f $(shell find client/src common/src server/src -type f -name '*.d')
	@rm -f $(shell find client/src common/src server/src -type f -name '*.wd')
	@rm -f $(shell find client/src common/src server/src -type f -name '*.deps')
	@rm -f wolkenwelten wolkenwelten.exe wolkenwelten-server wolkenwelten-server.exe tools/assets nujel nujel.exe
	@rm -f server/make.deps client/make.deps common/make.deps server/server.d client/client.d common/common.d nujel-standalone/nujel.d
	@rm -rf client/src/tmp server/src/tmp common/src/tmp web/releases releases nujel-standalone/tmp
	@echo "$(ANSI_BG_RED)" "[CLEAN]" "$(ANSI_RESET)" "wolkenwelten"

.PHONY: debug
debug: CFLAGS += -O0
debug: all

.PHONY: profile
profile: CFLAGS += -pg
profile: all

.PHONY: sanitize
sanitize: CFLAGS += -fsanitize=address
sanitize: all

.PHONY: archive
archive:
	git archive --format=tar --prefix=wolkenwelten-HEAD.tar.gz/ HEAD | gzip > wolkenwelten-HEAD.tar.gz

common/src/tmp/cto.c: tools/tools.nuj $(NUJEL)
	@mkdir -p common/src/tmp/
	@$(NUJEL) tools/tools.nuj -x "[try repl/exception-handler[infogen \"common/src/tmp/cto\"]]"
	@echo "$(ANSI_GREY)" "[NUJ]" "$(ANSI_RESET)" $@
