WIN_CLIENT_CFLAGS    := -DSDL_SOUND -mwindows -Wl,-subsystem,windows
WIN_CLIENT_CINCLUDES := -I/usr/x86_64-w64-mingw32/include/SDL2/

WIN_SERVER_LIBS      := -lmingw32 -static-libgcc -lws2_32 -luser32 -static -lpthread
WIN_SERVER_CFLAGS    :=

WINLIBS              := -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -lgdi32 -lopengl32 -lshlwapi -lsetupapi -lws2_32
WIN_STATICLIBS       := -static-libgcc -static -lmingw32 -lm -lstdc++ -lpthread -lopengl32 -lSDL2main -lssp -lSDL2 -lSDL2_mixer -lvorbis -logg -lvorbisfile $(WINLIBS)

WIN_RELDIR           := releases/win/wolkenwelten-win-$(VERSION_NAME)

.PHONY: release.win
release: release.win

release.win: releases/win/wolkenwelten-win-$(VERSION_NAME).7z
releases/win/wolkenwelten-win-$(VERSION_NAME).7z: $(WIN_RELDIR)/README.txt
releases/win/wolkenwelten-win-$(VERSION_NAME).7z: $(WIN_RELDIR)/wolkenwelten.exe
releases/win/wolkenwelten-win-$(VERSION_NAME).7z: $(WIN_RELDIR)/wolkenwelten-server.exe
	cd releases/win/ && 7z a wolkenwelten-win-$(VERSION_NAME).7z wolkenwelten-win-$(VERSION_NAME)/

$(WIN_RELDIR):
	mkdir -p $(WIN_RELDIR)

$(WIN_RELDIR)/README.txt: common/README
	cp $< $@

$(WIN_RELDIR)/wolkenwelten.exe: CFLAGS    += $(WIN_CLIENT_CFLAGS)
$(WIN_RELDIR)/wolkenwelten.exe: CINCLUDES += $(WIN_CLIENT_CINCLUDES)
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
	x86_64-w64-mingw32-gcc $(CLIENT_SRCS) $(ASM_OBJS) $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CINCLUDES) $(WIN_STATICLIBS) releases/win/wolkenwelten.res -o $@
	x86_64-w64-mingw32-strip -gxX $@

$(WIN_RELDIR)/wolkenwelten-server.exe: CFLAGS    += $(WIN_SERVER_CFLAGS)
$(WIN_RELDIR)/wolkenwelten-server.exe: CINCLUDES += $(WIN_SERVER_CINCLUDES)
$(WIN_RELDIR)/wolkenwelten-server.exe: $(SERVER_SRCS) $(SERVER_HDRS)
$(WIN_RELDIR)/wolkenwelten-server.exe: server/src/tmp/sfx.c server/src/tmp/sfx.h
$(WIN_RELDIR)/wolkenwelten-server.exe: server/src/tmp/objs.c server/src/tmp/objs.h
$(WIN_RELDIR)/wolkenwelten-server.exe: common/src/tmp/cto.c
$(WIN_RELDIR)/wolkenwelten-server.exe: releases/win/wolkenwelten-server.res
$(WIN_RELDIR)/wolkenwelten-server.exe: $(ASM_OBJS)
$(WIN_RELDIR)/wolkenwelten-server.exe: | $(WIN_RELDIR)
	x86_64-w64-mingw32-gcc $(SERVER_SRCS) $(ASM_OBJS) releases/win/wolkenwelten-server.res -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CINCLUDES) $(WIN_SERVER_LIBS)
	x86_64-w64-mingw32-strip -gxX $@

releases/win/wolkenwelten-server.res: | $(WIN_RELDIR)
releases/win/wolkenwelten-server.res: platform/win/wolkenwelten-server.rc platform/win/icon.ico
	x86_64-w64-mingw32-windres $< -O coff -o $@

releases/win/wolkenwelten.res: | $(WIN_RELDIR)
releases/win/wolkenwelten.res: platform/win/wolkenwelten.rc platform/win/icon.ico
	x86_64-w64-mingw32-windres $< -O coff -o $@
