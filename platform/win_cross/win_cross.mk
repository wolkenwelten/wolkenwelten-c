MINGW_CC             := x86_64-w64-mingw32-gcc
MINGW_STRIP          := x86_64-w64-mingw32-strip
MINGW_WINDRES        := x86_64-w64-mingw32-windres

WIN_CLIENT_CFLAGS    := -DSDL_SOUND -mwindows -Wl,-subsystem,windows
WIN_CLIENT_CINCLUDES := -I/usr/x86_64-w64-mingw32/include/SDL2/

WIN_SERVER_LIBS      := -lmingw32 -static-libgcc -lws2_32 -luser32 -static -lpthread
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

$(WIN_RELDIR)/README.txt: common/README
	cp $< $@

$(WIN_RELDIR)/wolkenwelten.exe: $(CLIENT_SRCS)
$(WIN_RELDIR)/wolkenwelten.exe: $(ASM_OBJS)
$(WIN_RELDIR)/wolkenwelten.exe: releases/win/wolkenwelten.res
	@mkdir -p $(WIN_RELDIR)
	$(MINGW_CC) $^ -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(WIN_CLIENT_CFLAGS) $(CINCLUDES) $(WIN_CLIENT_CINCLUDES) $(WIN_STATICLIBS)
	$(MINGW_STRIP) -gxX $@

$(WIN_RELDIR)/wolkenwelten-server.exe: $(SERVER_SRCS)
$(WIN_RELDIR)/wolkenwelten-server.exe: $(ASM_OBJS)
$(WIN_RELDIR)/wolkenwelten-server.exe: releases/win/wolkenwelten-server.res
	@mkdir -p $(WIN_RELDIR)
	$(MINGW_CC) $^ -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(WIN_SERVER_CFLAGS) $(CINCLUDES) $(WIN_SERVER_CINCLUDES) $(WIN_SERVER_LIBS)
	$(MINGW_STRIP) -gxX $@

releases/win/wolkenwelten-server.res: platform/win/wolkenwelten-server.rc platform/win/icon.ico
	$(MINGW_WINDRES) $< -O coff -o $@

releases/win/wolkenwelten.res: platform/win/wolkenwelten.rc platform/win/icon.ico
	$(MINGW_WINDRES) $< -O coff -o $@
