AS               := as
CC               := clang
LD               := clang

AS_SYM           := USE_GOT=USE_GOT

CFLAGS           += -D_GNU_SOURCE
LDFLAGS          += -D_GNU_SOURCE

CLIENT_CINCLUDES := $(shell sdl2-config --cflags)
CLIENT_LIBS      := -lGL -lpthread -ldl -lSDL2 -lSDL2_mixer

DYNLIBS          := -lGL -lm -lpthread -ldl -lSDL2 -lSDL2_mixer -lvorbis
STATICLIBS       :=

LIN_REL          := releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME)

.PHONY: all
all: wolkenwelten wolkenwelten-server

.PHONY:  release.linux

ifndef NOLINUXRELEASE
	release: release.linux
endif

$(LIN_REL):
	mkdir -p $(LIN_REL)

release.linux: releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz
releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz: $(LIN_REL)/README
releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz: $(LIN_REL)/wolkenwelten
releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz: $(LIN_REL)/wolkenwelten-server
	cd releases/linux-$(VERSION_ARCH)/ && tar -c wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME)/ | xz -9 - > wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz

$(LIN_REL)/README: common/README
	cp $< $@

$(LIN_REL)/wolkenwelten: CFLAGS    += $(CLIENT_CFLAGS)
$(LIN_REL)/wolkenwelten: CINCLUDES += $(CLIENT_CINCLUDES)
$(LIN_REL)/wolkenwelten: $(CLIENT_SRCS) $(CLIENT_HDRS)
$(LIN_REL)/wolkenwelten: client/src/tmp/assets.c client/src/tmp/assets.h
$(LIN_REL)/wolkenwelten: client/src/tmp/meshassets.c client/src/tmp/meshassets.h
$(LIN_REL)/wolkenwelten: client/src/tmp/objs.c client/src/tmp/objs.h
$(LIN_REL)/wolkenwelten: client/src/tmp/sfx.c client/src/tmp/sfx.h
$(LIN_REL)/wolkenwelten: common/src/tmp/cto.c common/src/tmp/cto.h
$(LIN_REL)/wolkenwelten: $(ASM_OBJS)
$(LIN_REL)/wolkenwelten: | $(LIN_REL)
	gcc $(CLIENT_SRCS) $(ASM_OBJS) -o $@ $(RELEASE_OPTIMIZATION) $(LDFLAGS) $(CSTD) $(CINCLUDES) $(DYNLIBS) $(STATICLIBS)
	strip -gxX $@

$(LIN_REL)/wolkenwelten-server: CFLAGS    += $(SERVER_CFLAGS)
$(LIN_REL)/wolkenwelten-server: CINCLUDES += $(SERVER_CINCLUDES)
$(LIN_REL)/wolkenwelten-server: $(SERVER_SRCS) $(SERVER_HDRS)
$(LIN_REL)/wolkenwelten-server: server/src/tmp/sfx.c server/src/tmp/sfx.h
$(LIN_REL)/wolkenwelten-server: server/src/tmp/objs.c server/src/tmp/objs.h
$(LIN_REL)/wolkenwelten-server: common/src/tmp/cto.c common/src/tmp/cto.h
$(LIN_REL)/wolkenwelten-server: $(ASM_OBJS)
$(LIN_REL)/wolkenwelten-server: | $(LIN_REL)
	musl-gcc -static $(SERVER_SRCS) $(ASM_OBJS) -o $@ $(RELEASE_OPTIMIZATION) $(LDFLAGS) $(CSTD) $(CINCLUDES)
	strip -gxX $@
