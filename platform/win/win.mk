AS               := /mingw64/bin/as
CC               := /mingw64/bin/gcc
LD               := /mingw64/bin/gcc

CLIENT_CFLAGS    := -DSDL_SOUND -mwindows -Wl,-subsystem,windows $(shell pkg-config --cflags sdl2 sdl2_mixer)

NUJEL_LIBS       := -static-libgcc -lws2_32 -luser32 -static -lpthread

SERVER_LIBS      := -lmingw32 -static-libgcc -lws2_32 -luser32 -static -lpthread
SERVER_CFLAGS    :=

WINLIBS          := -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -lgdi32 -lopengl32 -lshlwapi -lsetupapi -lws2_32
STATICLIBS       := -static-libgcc -static -lmingw32 -lm -lstdc++ -lpthread -lopengl32 -lSDL2main -lSDL2 -lSDL2_mixer -lvorbis -logg -lvorbisfile $(WINLIBS)
CLIENT_LIBS      := $(WINLIBS) $(shell pkg-config --cflags sdl2 sdl2_mixer) -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lvorbis

WIN_RELDIR       := releases/win/wolkenwelten-win-$(VERSION_NAME)

.PHONY: all
all: wolkenwelten.exe wolkenwelten-server.exe nujel.exe

.PHONY: release.win
release: release.win

$(WIN_RELDIR):
	mkdir -p $(WIN_RELDIR)

release.win: releases/win/wolkenwelten-win-$(VERSION_NAME).7z
releases/win/wolkenwelten-win-$(VERSION_NAME).7z: $(WIN_RELDIR)/README.txt
releases/win/wolkenwelten-win-$(VERSION_NAME).7z: $(WIN_RELDIR)/wolkenwelten.exe
releases/win/wolkenwelten-win-$(VERSION_NAME).7z: $(WIN_RELDIR)/wolkenwelten-server.exe
	cd releases/win/ && 7z a wolkenwelten-win-$(VERSION_NAME).7z wolkenwelten-win-$(VERSION_NAME)

$(WIN_RELDIR)/README.txt: common/README
	cp $< $@

$(WIN_RELDIR)/wolkenwelten.exe: CFLAGS    += $(CLIENT_CFLAGS)
$(WIN_RELDIR)/wolkenwelten.exe: CINCLUDES += $(CLIENT_CINCLUDES)
$(WIN_RELDIR)/wolkenwelten.exe: $(CLIENT_SRCS) $(CLIENT_HDRS)
$(WIN_RELDIR)/wolkenwelten.exe: client/src/tmp/gfxAssets.c client/src/tmp/gfxAssets.h
$(WIN_RELDIR)/wolkenwelten.exe: client/src/tmp/sfxAssets.c client/src/tmp/sfxAssets.h
$(WIN_RELDIR)/wolkenwelten.exe: client/src/tmp/shdAssets.c client/src/tmp/shdAssets.h
$(WIN_RELDIR)/wolkenwelten.exe: client/src/tmp/txtAssets.c client/src/tmp/txtAssets.h
$(WIN_RELDIR)/wolkenwelten.exe: client/src/tmp/nujAssets.c client/src/tmp/nujAssets.h
$(WIN_RELDIR)/wolkenwelten.exe: client/src/tmp/meshAssets.c client/src/tmp/meshAssets.h
$(WIN_RELDIR)/wolkenwelten.exe: client/src/tmp/objs.c client/src/tmp/objs.h
$(WIN_RELDIR)/wolkenwelten.exe: client/src/tmp/sfx.c client/src/tmp/sfx.h
$(WIN_RELDIR)/wolkenwelten.exe: common/src/tmp/cto.c
$(WIN_RELDIR)/wolkenwelten.exe: releases/win/wolkenwelten.res
$(WIN_RELDIR)/wolkenwelten.exe: $(ASM_OBJS)
$(WIN_RELDIR)/wolkenwelten.exe: | $(WIN_RELDIR)
	$(CC) $(CLIENT_SRCS) $(ASM_OBJS) $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CINCLUDES) $(STATICLIBS) releases/win/wolkenwelten.res -o $@
	strip -gxX $@

$(WIN_RELDIR)/wolkenwelten-server.exe: CFLAGS    += $(SERVER_CFLAGS)
$(WIN_RELDIR)/wolkenwelten-server.exe: CINCLUDES += $(SERVER_CINCLUDES)
$(WIN_RELDIR)/wolkenwelten-server.exe: LIBS      += $(SERVER_LIBS)
$(WIN_RELDIR)/wolkenwelten-server.exe: $(SERVER_SRCS) $(SERVER_HDRS)
$(WIN_RELDIR)/wolkenwelten-server.exe: server/src/tmp/sfx.c server/src/tmp/sfx.h
$(WIN_RELDIR)/wolkenwelten-server.exe: server/src/tmp/objs.c server/src/tmp/objs.h
$(WIN_RELDIR)/wolkenwelten-server.exe: common/src/tmp/cto.c
$(WIN_RELDIR)/wolkenwelten-server.exe: releases/win/wolkenwelten-server.res
$(WIN_RELDIR)/wolkenwelten-server.exe: | $(WIN_RELDIR)
	$(CC) $(SERVER_SRCS) $(ASM_OBJS) releases/win/wolkenwelten-server.res -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CINCLUDES) $(LIBS)
	strip -gxX $@

wolkenwelten.exe: CFLAGS    += $(CLIENT_CFLAGS)
wolkenwelten.exe: CINCLUDES += $(CLIENT_CINCLUDES)
wolkenwelten.exe: LIBS      += $(CLIENT_LIBS)
wolkenwelten.exe: client/client.d releases/win/wolkenwelten.res $(CLIENT_OBJS) $(ASM_OBJS)
	rm -f wolkenwelten.exe
	$(CC) $(CLIENT_OBJS) $(ASM_OBJS) releases/win/wolkenwelten.res -g -o $@ $(OPTIMIZATION) $(CFLAGS) $(CINCLUDES) $(LIBS)

wolkenwelten-server.exe: CFLAGS    += $(SERVER_CFLAGS)
wolkenwelten-server.exe: CINCLUDES += $(SERVER_CINCLUDES)
wolkenwelten-server.exe: LIBS      += $(SERVER_LIBS)
wolkenwelten-server.exe: server/server.d releases/win/wolkenwelten-server.res $(SERVER_OBJS) $(ASM_OBJS)
	rm -f wolkenwelten-server.exe
	$(CC) $(SERVER_OBJS) $(ASM_OBJS) releases/win/wolkenwelten-server.res -g -o $@ $(OPTIMIZATION) $(CFLAGS) $(CINCLUDES) $(LIBS)

releases/win/wolkenwelten-server.res: | $(WIN_RELDIR)
releases/win/wolkenwelten-server.res: platform/win/wolkenwelten-server.rc platform/win/icon.ico
	windres $< -O coff -o $@

releases/win/wolkenwelten.res: | $(WIN_RELDIR)
releases/win/wolkenwelten.res: platform/win/wolkenwelten.rc platform/win/icon.ico
	windres $< -O coff -o $@
