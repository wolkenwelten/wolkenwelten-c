FFMPEG           := ffmpeg
ifeq (, $(shell which ffmpeg))
	FFMPEG   := ffmpeg4
endif
CLIENT_CFLAGS    :=
CLIENT_CINCLUDES :=
CLIENT_LIBS      :=

GFX_ASSETS       := $(shell find client/gfx -type f -name '*')
RAW_SFX          := $(shell find client/sfx -type f -name '*.aif')
SFX_ASSETS       := ${RAW_SFX:.aif=.ogg}
SHD_ASSETS       := $(shell find client/src/shader -type f -name '*.glsl')
TXT_ASSETS       := $(shell find client/txt -type f -name '*')
CLIENT_ASSETS    := $(GFX_ASSETS) $(SFX_ASSETS) $(SHD_ASSETS) $(TXT_ASSETS) client/src/tmp/client.nuj
MESHASSETS       := $(shell find client/mesh -type f -name '*')
CLIENT_NUJ       := $(shell find client/src/nujel/ -type f -name '*.nuj' | sort)

CLIENT_HDRS      := $(shell find client/src -type f -name '*.h') $(COMMON_HDRS)
CLIENT_SRCS      := $(shell find client/src -type f -name '*.c') $(COMMON_SRCS)
CLIENT_OBJS      := ${CLIENT_SRCS:.c=.o}
CLIENT_DEPS      := ${CLIENT_SRCS:.c=.d}

WINDOW_WIDTH     := 960
WINDOW_HEIGHT    := 524
TESTNR           := 1

client/src/menu/singleplayer.o: common/src/tmp/cto.h
client/src/menu/mainmenu.o:     common/src/tmp/cto.h
client/src/menu/multiplayer.o:  common/src/tmp/cto.h
client/src/menu/options.o:      common/src/tmp/cto.h
client/src/misc/options.o:      common/src/tmp/cto.h
client/src/gui/menu.o:          common/src/tmp/cto.h
client/src/gui/gui.o:           common/src/tmp/cto.h
client/src/gfx/gfx.o:           common/src/tmp/assets.h
client/src/gfx/sky.o:           common/src/tmp/assets.h
client/src/gfx/shadow.o:        common/src/tmp/assets.h
client/src/gfx/textMesh.o:      common/src/tmp/assets.h
client/src/gfx/particle.o:      common/src/tmp/assets.h
client/src/gfx/mesh.o:          common/src/tmp/assets.h
client/src/gfx/shader.o:        common/src/tmp/assets.h
client/src/gfx/texture.o:       common/src/tmp/assets.h
client/src/game/blockMining.o:  client/src/tmp/assets.h
client/src/main.o:              client/src/tmp/sfx.h
client/src/game/character.o:    client/src/tmp/sfx.h
client/src/gfx/effects.o:       client/src/tmp/sfx.h
client/src/sdl/sfx.o:           client/src/tmp/sfx.h
client/src/sdl/sdl.o:           client/src/tmp/sfx.h
client/src/game/entity.o:       client/src/tmp/sfx.h
client/src/game/fire.o:         client/src/tmp/sfx.h
client/src/game/projectile.o:   client/src/tmp/sfx.h
client/src/misc/lisp.o:         client/src/tmp/sfx.h
client/src/menu/inventory.o:    client/src/tmp/sfx.h

wolkenwelten: CFLAGS    += $(CLIENT_CFLAGS)
wolkenwelten: CINCLUDES += $(CLIENT_CINCLUDES)
wolkenwelten: LIBS      += $(CLIENT_LIBS)
wolkenwelten: client/client.d $(CLIENT_OBJS) $(ASM_OBJS)
	$(LD) $(CLIENT_OBJS) $(ASM_OBJS) -g -o wolkenwelten $(OPTIMIZATION) $(LDFLAGS) $(LIBS) $(CSTD)

$(CLIENT_DEPS): | client/src/tmp/client.nuj client/src/tmp/assets.h client/src/tmp/meshassets.h common/src/tmp/cto.h client/src/tmp/objs.h client/src/tmp/sfx.h
.deps: client/client.d
client/client.d: $(CLIENT_DEPS)
	cat $(CLIENT_DEPS) > client/client.d

client/src/tmp/client.nuj: $(CLIENT_NUJ)
	@mkdir -p client/src/tmp
	cat $(CLIENT_NUJ) > $@

ifneq ($(MAKECMDGOALS),clean)
-include client/client.d
endif

%.ogg: %.aif
	$(FFMPEG) -hide_banner -v panic -i $< -ac 1 -ar 22050 -acodec libvorbis $@

client/src/tmp/assets.c: $(ASSET) $(CLIENT_ASSETS)
	@mkdir -p client/src/tmp/
	./$(ASSET) client/src/tmp/assets $(CLIENT_ASSETS)
client/src/tmp/assets.h: client/src/tmp/assets.c
	@true

client/src/tmp/meshassets.c: client/tools/objparser $(MESHASSETS)
	@mkdir -p client/src/tmp/
	client/tools/objparser $(MESHASSETS)
client/src/tmp/meshassets.h: client/src/tmp/meshassets.c
	@true

client/tools/objparser: client/tools/objparser.c
	$(CC) $(OPTIMIZATION) $(CFLAGS) $(CSTD) $< -o $@

client/src/tmp/objs.c: $(MESHASSETS)
client/src/tmp/objs.c: client/tools/objgen
	@mkdir -p client/src/tmp/
	client/tools/objgen client/src/tmp/objs client/mesh/
client/src/tmp/objs.h: client/src/tmp/objs.c
	@true

client/src/tmp/sfx.c: $(SFX_ASSETS)
client/src/tmp/sfx.c: client/tools/sfxgen
	@mkdir -p client/src/tmp/
	client/tools/sfxgen client/src/tmp/sfx client/sfx/
client/src/tmp/sfx.h: client/src/tmp/sfx.c
	@true

.PHONY: optimizegfx
optimizegfx: all
	client/tools/optimizegfx $(GFX_ASSETS)

.PHONY: run
run: all
	./wolkenwelten -soundVolume=10 $(TEST_WORLD) -debugInfo=1 -fullscreen
.PHONY: runcd
runcd: all
	gdb ./wolkenwelten -ex "r -soundVolume=10 $(TEST_WORLD) -debugInfo=1"
.PHONY: runt
runt: all
	rm -rf save/
	./wolkenwelten -soundVolume=10 $(TEST_WORLD) -debugInfo=1 -windowOrientation=cc -windowWidth=$(WINDOW_WIDTH) -windowHeight=$(WINDOW_HEIGHT) -automatedTest=$(TESTNR)
runtv: all
	rm -rf save/
	rm -f ./callgrind.out.*
	valgrind --tool=callgrind ./wolkenwelten-server $(TEST_WORLD) -singleplayer &
	./wolkenwelten -soundVolume=10  -serverName=localhost -debugInfo=1 -windowOrientation=cc -windowWidth=$(WINDOW_WIDTH) -windowHeight=$(WINDOW_HEIGHT) -automatedTest=$(TESTNR)
.PHONY: runl
runl: all
	./wolkenwelten -soundVolume=10 -debugInfo=1 -serverName=localhost
runld: all
	gdb ./wolkenwelten -ex "r -soundVolume=10 -debugInfo=1 -serverName=localhost"
.PHONY: rund
rund: all
	./wolkenwelten-server $(TEST_WORLD) -singleplayer &
	./wolkenwelten -noSave -soundVolume=0  -debugInfo=1 -serverName=localhost -playerName=Captain     -windowOrientation=tl -windowWidth=$(WINDOW_WIDTH) -windowHeight=$(WINDOW_HEIGHT) &
	./wolkenwelten -noSave -soundVolume=10 -debugInfo=1 -serverName=localhost -playerName=NumberTwo   -windowOrientation=bl -windowWidth=$(WINDOW_WIDTH) -windowHeight=$(WINDOW_HEIGHT) &
.PHONY: runq
runq: all
	./wolkenwelten-server $(TEST_WORLD) -singleplayer &
	./wolkenwelten -noSave -soundVolume=0  -debugInfo=1 -serverName=localhost -playerName=Captain     -windowOrientation=tl -windowWidth=$(WINDOW_WIDTH) -windowHeight=$(WINDOW_HEIGHT) &
	./wolkenwelten -noSave -soundVolume=0  -debugInfo=1 -serverName=localhost -playerName=NumberTwo   -windowOrientation=tr -windowWidth=$(WINDOW_WIDTH) -windowHeight=$(WINDOW_HEIGHT) &
	./wolkenwelten -noSave -soundVolume=0  -debugInfo=1 -serverName=localhost -playerName=NumberThree -windowOrientation=bl -windowWidth=$(WINDOW_WIDTH) -windowHeight=$(WINDOW_HEIGHT) &
	./wolkenwelten -noSave -soundVolume=10 -debugInfo=1 -serverName=localhost -playerName=NumberFour  -windowOrientation=br -windowWidth=$(WINDOW_WIDTH) -windowHeight=$(WINDOW_HEIGHT) &

client/src/network/client.o: client/src/network/client_win.h
client/src/network/client.o: client/src/network/client_bsd.h
