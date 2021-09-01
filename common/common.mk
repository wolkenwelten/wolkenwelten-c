ARCH             := $(shell uname -m)
NUJ_STDLIB       := $(shell find common/src/nujel/stdlib/ -type f -name '*.nuj' | sort)
NUJ_WWLIB        := $(shell find common/src/nuj/ -type f -name '*.nuj' | sort)
COMMON_ASSETS    := common/src/tmp/stdlib.nuj common/src/tmp/wwlib.nuj
COMMON_HDRS      := $(shell find common/src -type f -name '*.h')
COMMON_SRCS      := $(shell find common/src -type f -name '*.c')
COMMON_OBJS      := ${COMMON_SRCS:.c=.o}
COMMON_DEPS      := ${COMMON_SRCS:.c=.d}
ASM_OBJS         :=

WEBEXCLUDE       += --exclude=releases/macos/wolkenwelten.iconset/
WEBEXCLUDE       += --exclude=releases/win/*.res

TEST_WORLD       := -worldSeed=68040 -savegame=Test

ASM_SRCS         := common/src/asm/$(ARCH).s

ifneq ("$(wildcard $(ASM_SRCS))","")
	ASM_OBJS := ${ASM_SRCS:.s=.o}
else
	ASM_SRCS :=
endif

$(COMMON_OBJS): | client/src/tmp/objs.h
$(COMMON_OBJS): | client/src/tmp/sfx.h
$(COMMON_OBJS): | common/src/tmp/assets.h
$(COMMON_OBJS): | server/src/tmp/assets.h
$(COMMON_OBJS): | server/src/tmp/objs.h
$(COMMON_OBJS): | server/src/tmp/sfx.h

%.o: %.s
	$(AS) $(ASFLAGS) -c --defsym $(AS_SYM) $< -o $@

%.o: %.c
	$(CC) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(CFLAGS) $(CINCLUDES) $(if $(findstring client/, $<),$(CLIENT_CFLAGS) $(CLIENT_CINCLUDES),) -g -c $< -o $@ -MMD > ${<:.c=.d}

common/src/tmp/stdlib.nuj: $(NUJ_STDLIB)
	@mkdir -p common/src/tmp
	cat $(NUJ_STDLIB) > $@

common/src/tmp/wwlib.nuj: $(NUJ_WWLIB)
	@mkdir -p common/src/tmp
	cat $(NUJ_WWLIB) > $@

common/src/tmp/assets.c: $(ASSET) $(COMMON_ASSETS)
	@mkdir -p common/src/tmp/
	$(ASSET) common/src/tmp/assets $(COMMON_ASSETS)
common/src/tmp/assets.h: common/src/tmp/assets.c
	@true

$(ASSET): tools/assets.c
	$(CC) $(OPTIMIZATION) $(CSTD) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f gmon.out client/tools/assets client/tools/objparser callgrind.out.* vgcore.* platform/win/wolkenwelten.res
	rm -f client/sfx/*.ogg
	rm -f $(shell find client/src common/src nujel-standalone/ server/src -type f -name '*.o')
	rm -f $(shell find client/src common/src nujel-standalone/ server/src -type f -name '*.wo')
	rm -f $(shell find client/src common/src nujel-standalone/ server/src -type f -name '*.d')
	rm -f $(shell find client/src common/src nujel-standalone/ server/src -type f -name '*.wd')
	rm -f $(shell find client/src common/src nujel-standalone/ server/src -type f -name '*.deps')
	rm -f wolkenwelten wolkenwelten.exe wolkenwelten-server wolkenwelten-server.exe tools/assets nujel nujel.exe
	rm -f server/make.deps client/make.deps common/make.deps server/server.d client/client.d common/common.d nujel-standalone/nujel.d
	rm -rf client/src/tmp server/src/tmp common/src/tmp web/releases releases nujel-standalone/tmp

.PHONY: web
web: release
	rsync -avhe ssh web/ wolkenwelten.net:/var/www/html/
	ssh wolkenwelten.net "mkdir -p /var/www/html/releases && cd /var/www/html/releases && mkdir -p win macos linux-x86_64 linux-aarch64 linux-armv7l"
	if [ -d "releases/wasm" ]; then rsync -avhe ssh releases/wasm/ wolkenwelten.net:/var/www/html/releases/wasm/ ; fi
	if [ -d "releases/win" ]; then rsync -avhe ssh releases/win/*.7z wolkenwelten.net:/var/www/html/releases/win/ ; fi
	if [ -d "releases/macos" ]; then rsync -avhe ssh releases/macos/*.tgz wolkenwelten.net:/var/www/html/releases/macos/ ; fi
	if [ -d "releases/linux-armv7l" ]; then rsync -avhe ssh releases/linux-armv7l/*.xz wolkenwelten.net:/var/www/html/releases/linux-armv7l/ ; fi
	if [ -d "releases/linux-aarch64" ]; then rsync -avhe ssh releases/linux-aarch64/*.xz wolkenwelten.net:/var/www/html/releases/linux-aarch64/ ; fi
	if [ -d "releases/linux-x86_64" ]; then rsync -avhe ssh releases/linux-x86_64/*.xz wolkenwelten.net:/var/www/html/releases/linux-x86_64/ ; fi
	ssh wolkenwelten.net "cd /var/www/html/ && guile template.scm"

.PHONY: website
website:
	rsync -avhe ssh web/ wolkenwelten.net:/var/www/html/
	ssh wolkenwelten.net "cd /var/www/html/ && guile template.scm"

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
	$(NUJEL) tools/tools.nuj -x "[infogen \"common/src/tmp/cto\"]"
