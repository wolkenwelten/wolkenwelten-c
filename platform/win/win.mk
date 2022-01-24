AS               := /mingw64/bin/as
CC               := /mingw64/bin/gcc

CLIENT_CFLAGS    := -DSDL_SOUND -mwindows -Wl,-subsystem,windows $(shell pkg-config --cflags sdl2 sdl2_mixer)

NUJEL_LIBS       := -static-libgcc -lws2_32 -luser32 -static -lpthread

SERVER_LIBS      := -lmingw32 -static-libgcc -lws2_32 -luser32 -static -lpthread

WINLIBS          := -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -lgdi32 -lopengl32 -lshlwapi -lsetupapi -lws2_32
STATICLIBS       := -static-libgcc -static -lmingw32 -lm -lstdc++ -lpthread -lopengl32 -lSDL2main -lSDL2 -lSDL2_mixer -lvorbis -logg -lvorbisfile $(WINLIBS)
CLIENT_LIBS      := $(WINLIBS) $(shell pkg-config --cflags sdl2 sdl2_mixer) $(shell pkg-config --libs sdl2 sdl2_mixer) -lvorbis -lopengl32

WIN_RELDIR       := releases/win/wolkenwelten-win-$(VERSION_NAME)

.PHONY: release.win
release: release.win


release.win: releases/win/wolkenwelten-win-$(VERSION_NAME).7z
releases/win/wolkenwelten-win-$(VERSION_NAME).7z: $(WIN_RELDIR)/README.txt
releases/win/wolkenwelten-win-$(VERSION_NAME).7z: $(WIN_RELDIR)/wolkenwelten.exe
releases/win/wolkenwelten-win-$(VERSION_NAME).7z: $(WIN_RELDIR)/wolkenwelten-server.exe
	cd releases/win/ && 7z a wolkenwelten-win-$(VERSION_NAME).7z wolkenwelten-win-$(VERSION_NAME)

$(WIN_RELDIR)/README.txt: common/README
	@mkdir -p $(WIN_RELDIR)
	cp $< $@

$(WIN_RELDIR)/wolkenwelten.exe: $(CLIENT_SRCS) $(CLIENT_TMP_SRCS)
$(WIN_RELDIR)/wolkenwelten.exe: $(ASM_OBJS) common/nujel/nujel.a common/nujel/tmp/stdlib.o
$(WIN_RELDIR)/wolkenwelten.exe: releases/win/wolkenwelten.res
	@mkdir -p $(WIN_RELDIR)
	$(CC) $^ -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CLIENT_CFLAGS) $(CINCLUDES) $(CLIENT_CINCLUDES) $(STATICLIBS)
	strip -gxX $@

$(WIN_RELDIR)/wolkenwelten-server.exe: $(SERVER_SRCS) $(SERVER_TMP_SRCS)
$(WIN_RELDIR)/wolkenwelten-server.exe: $(ASM_OBJS) common/nujel/nujel.a common/nujel/tmp/stdlib.o
$(WIN_RELDIR)/wolkenwelten-server.exe: releases/win/wolkenwelten-server.res
	@mkdir -p $(WIN_RELDIR)
	$(CC) $^ -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(SERVER_CFLAGS) $(CINCLUDES) $(SERVER_CINCLUDES) $(LIBS) $(SERVER_LIBS)
	strip -gxX $@

$(WOLKENWELTEN): releases/win/wolkenwelten.res
$(WOLKENWELTEN_SERVER): releases/win/wolkenwelten-server.res

releases/win/wolkenwelten-server.res: platform/win/wolkenwelten-server.rc platform/win/icon.ico
	@mkdir -p $(WIN_RELDIR)
	windres $< -O coff -o $@
releases/win/wolkenwelten.res: platform/win/wolkenwelten.rc platform/win/icon.ico
	@mkdir -p $(WIN_RELDIR)
	windres $< -O coff -o $@
