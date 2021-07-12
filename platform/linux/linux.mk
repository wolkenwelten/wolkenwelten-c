AS               := as
CC               := gcc

AS_SYM           := USE_GOT=USE_GOT

CLIENT_LIBS      := -lGL -lm -lpthread -ldl -lSDL2 -lSDL2_mixer -lvorbis
DYNLIBS          := -lGL -lm -lpthread -ldl -lSDL2 -lSDL2_mixer -lvorbis
STATICLIBS       :=
SERVER_LIBS      := -lm

LIN_REL          := releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME)

.PHONY: all
all: wolkenwelten wolkenwelten-server

.PHONY:  release.linux

ifndef NOLINUXRELEASE
	release: release.linux
endif

release.linux: releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz
releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz: $(LIN_REL)/README
releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz: $(LIN_REL)/wolkenwelten
releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz: $(LIN_REL)/wolkenwelten-server
	cd releases/linux-$(VERSION_ARCH)/ && tar -c wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME)/ | xz -9 - > wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz

$(LIN_REL)/README: common/README
	@mkdir -p $(LIN_REL)
	cp $< $@

$(LIN_REL)/wolkenwelten: CFLAGS    += $(CLIENT_CFLAGS)
$(LIN_REL)/wolkenwelten: CINCLUDES += $(CLIENT_CINCLUDES)
$(LIN_REL)/wolkenwelten: $(CLIENT_SRCS) $(CLIENT_HDRS)
$(LIN_REL)/wolkenwelten: client/src/tmp/gfxAssets.c  client/src/tmp/gfxAssets.h
$(LIN_REL)/wolkenwelten: client/src/tmp/sfxAssets.c  client/src/tmp/sfxAssets.h
$(LIN_REL)/wolkenwelten: client/src/tmp/shdAssets.c  client/src/tmp/shdAssets.h
$(LIN_REL)/wolkenwelten: client/src/tmp/txtAssets.c  client/src/tmp/txtAssets.h
$(LIN_REL)/wolkenwelten: client/src/tmp/nujAssets.c  client/src/tmp/nujAssets.h
$(LIN_REL)/wolkenwelten: client/src/tmp/meshAssets.c client/src/tmp/meshAssets.h
$(LIN_REL)/wolkenwelten: client/src/tmp/objs.c client/src/tmp/objs.h
$(LIN_REL)/wolkenwelten: client/src/tmp/sfx.c client/src/tmp/sfx.h
$(LIN_REL)/wolkenwelten: common/src/tmp/cto.c
$(LIN_REL)/wolkenwelten: $(ASM_OBJS)
	@mkdir -p $(LIN_REL)
	gcc $(CLIENT_SRCS) $(ASM_OBJS) -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CSTD) $(CINCLUDES) $(DYNLIBS) $(STATICLIBS)
	strip -gxX $@

$(LIN_REL)/wolkenwelten-server: CFLAGS    += $(SERVER_CFLAGS)
$(LIN_REL)/wolkenwelten-server: CINCLUDES += $(SERVER_CINCLUDES)
$(LIN_REL)/wolkenwelten-server: $(SERVER_SRCS) $(SERVER_HDRS)
$(LIN_REL)/wolkenwelten-server: server/src/tmp/sfx.c server/src/tmp/sfx.h
$(LIN_REL)/wolkenwelten-server: server/src/tmp/objs.c server/src/tmp/objs.h
$(LIN_REL)/wolkenwelten-server: common/src/tmp/cto.c
$(LIN_REL)/wolkenwelten-server: $(ASM_OBJS)
	@mkdir -p $(LIN_REL)
	musl-gcc -static  $(SERVER_SRCS) $(ASM_OBJS) -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CSTD) $(CINCLUDES)
	strip -gxX $@
