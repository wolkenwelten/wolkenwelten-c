AS               := as
CC               := gcc
STRIP            := strip
MUSL_CC          := musl-gcc

AS_SYM           := --defsym USE_GOT=USE_GOT

GL_LIBS          := $(shell pkg-config --silence-errors --libs gl || pkg-config --libs opengl)

CLIENT_LIBS      := $(GL_LIBS) -lm -lpthread -ldl -lSDL2 -lSDL2_mixer -lvorbis
SERVER_LIBS      := -lm
LIN_REL          := releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME)

.PHONY:  release.linux
release: release.linux

release.linux: releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz
releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz: $(LIN_REL)/README
releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz: $(LIN_REL)/wolkenwelten
releases/linux-$(VERSION_ARCH)/wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz: $(LIN_REL)/wolkenwelten-server
	cd releases/linux-$(VERSION_ARCH)/ && tar -c wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME)/ | xz -9 - > wolkenwelten-linux-$(VERSION_ARCH)-$(VERSION_NAME).tar.xz

$(LIN_REL)/README: common/README
	@mkdir -p $(LIN_REL)
	cp $< $@

$(LIN_REL)/wolkenwelten: $(CLIENT_SRCS) $(CLIENT_TMP_SRCS) $(ASM_OBJS) common/nujel/nujel.a
	@mkdir -p $(LIN_REL)
	$(CC) $^ -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CLIENT_CFLAGS) $(CSTD) $(CINCLUDES) $(CLIENT_CINCLUDES) $(CLIENT_LIBS)
	$(STRIP) -gxX $@

$(LIN_REL)/wolkenwelten-server: $(SERVER_SRCS) $(SERVER_TMP_SRCS) $(ASM_OBJS) common/nujel/nujel.a
	@mkdir -p $(LIN_REL)
	$(CC) $^ -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(SERVER_CFLAGS) $(CSTD) $(CINCLUDES) $(SERVER_CINCLUDES) $(SERVER_LIBS)
	$(STRIP) -gxX $@
