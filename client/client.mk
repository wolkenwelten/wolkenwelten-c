FFMPEG            := ffmpeg
ifeq (, $(shell which ffmpeg))
	FFMPEG    := ffmpeg4
endif
CLIENT_CFLAGS     := $(shell pkg-config --cflags sdl2 SDL2_mixer)
CLIENT_LIBS       := $(shell pkg-config --libs sdl2 SDL2_mixer)

GFX_ASSETS        := $(shell find client/gfx -type f -name '*')
RAW_SFX           := $(shell find client/sfx -type f -name '*.aif')
RAW_WAV           := $(shell find client/sfx -type f -name '*.wav')
SFX_ASSETS        := ${RAW_SFX:.aif=.ogg} ${RAW_WAV:.wav=.ogg}
SHD_ASSETS        := $(shell find client/src/shader -type f -name '*.glsl')
TXT_ASSETS        := $(shell find client/txt -type f -name '*')
MESHASSETS        := $(shell find client/mesh -type f -name '*')
CLIENT_NUJ        := $(shell find client/src/nujel/ -type f -name '*.nuj' | sort)
CLIENT_NO         := $(CLIENT_NUJ:.nuj=.no)
CLIENT_NUJ_ASSETS := client/src/tmp/client.nuj
CLIENT_TMP_SRCS   := client/src/tmp/objs.c client/src/tmp/gfxAssets.c client/src/tmp/sfxAssets.c client/src/tmp/shdAssets.c client/src/tmp/txtAssets.c client/src/tmp/nujAssets.c client/src/tmp/meshAssets.c common/src/tmp/assets.c common/src/tmp/cto.c client/src/tmp/sfx.c
CLIENT_TMP_OBJS   := ${CLIENT_TMP_SRCS:.c=.o}

CLIENT_HDRS       := $(shell find client/src -type f -name '*.h') $(COMMON_HDRS)
CLIENT_SRCS_EXCL  := $(shell find client/src -type f -name '*.c')
CLIENT_SRCS       := $(CLIENT_SRCS_EXCL) $(COMMON_SRCS)
CLIENT_OBJS_EXCL  := ${CLIENT_SRCS_EXCL:.c=.o}
CLIENT_OBJS       := $(CLIENT_OBJS_EXCL) ${COMMON_SRCS:.c=.o}
CLIENT_DEPS       := ${CLIENT_SRCS:.c=.d}

WINDOW_WIDTH      := 960
WINDOW_HEIGHT     := 524
TESTNR            := 1

$(CLIENT_OBJS_EXCL): | common/src/tmp/cto.c
$(CLIENT_OBJS):      | client/src/tmp/objs.h
$(CLIENT_OBJS):      | client/src/tmp/sfx.h

$(WOLKENWELTEN): $(CLIENT_OBJS) $(ASM_OBJS) $(CLIENT_TMP_OBJS) common/nujel/nujel.a common/nujel/tmp/stdlib.o
	@$(CC) $^ -g -o $@ $(OPTIMIZATION) $(CLIENT_CFLAGS) $(CFLAGS) $(CLIENT_CINCLUDES) $(CINCLUDES) $(CLIENT_LIBS) $(CSTD)
	@echo "$(ANSI_BG_GREEN)" "[LD] " "$(ANSI_RESET)" $@

client/src/tmp/client.no: $(CLIENT_NO)
	@mkdir -p client/src/tmp
	@cat $^ > $@
	@echo "$(ANSI_GREY)" "[CAT]" "$(ANSI_RESET)" $@

client/src/tmp/client.nuj: $(CLIENT_NUJ)
	@mkdir -p client/src/tmp
	@cat $^ > $@
	@echo "$(ANSI_GREY)" "[CAT]" "$(ANSI_RESET)" $@

%.ogg: %.aif
	@$(FFMPEG) -hide_banner -y -v panic -i $< -ac 1 -ar 22050 -acodec libvorbis $@ < /dev/null
	@echo "$(ANSI_CYAN)" "[FF] " "$(ANSI_RESET)" $@

%.ogg: %.wav
	@$(FFMPEG) -hide_banner -y -v panic -i $< -ac 1 -ar 22050 -acodec libvorbis $@ < /dev/null
	@echo "$(ANSI_CYAN)" "[FF] " "$(ANSI_RESET)" $@

client/src/tmp/gfxAssets.c: $(ASSET) $(GFX_ASSETS)
	@mkdir -p client/src/tmp/
	@$(ASSET) client/src/tmp/gfxAssets $(GFX_ASSETS)
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
client/src/tmp/gfxAssets.h: client/src/tmp/gfxAssets.c
	@true

client/src/tmp/sfxAssets.c: $(ASSET) $(SFX_ASSETS)
	@mkdir -p client/src/tmp/
	@$(ASSET) client/src/tmp/sfxAssets $(SFX_ASSETS)
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
client/src/tmp/sfxAssets.h: client/src/tmp/sfxAssets.c
	@true

client/src/tmp/shdAssets.c: $(ASSET) $(SHD_ASSETS)
	@mkdir -p client/src/tmp/
	@$(ASSET) client/src/tmp/shdAssets $(SHD_ASSETS)
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
client/src/tmp/shdAssets.h: client/src/tmp/shdAssets.c
	@true

client/src/tmp/txtAssets.c: $(ASSET) $(TXT_ASSETS)
	@mkdir -p client/src/tmp/
	@$(ASSET) client/src/tmp/txtAssets $(TXT_ASSETS)
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
client/src/tmp/txtAssets.h: client/src/tmp/txtAssets.c
	@true

client/src/tmp/nujAssets.c: $(ASSET) $(CLIENT_NUJ_ASSETS)
	@mkdir -p client/src/tmp/
	@$(ASSET) client/src/tmp/nujAssets $(CLIENT_NUJ_ASSETS)
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
client/src/tmp/nujAssets.h: client/src/tmp/nujAssets.c
	@true

client/src/tmp/meshAssets.c: client/tools/objparser $(MESHASSETS)
	@mkdir -p client/src/tmp/
	@client/tools/objparser $(MESHASSETS)
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
client/src/tmp/meshAssets.h: client/src/tmp/meshAssets.c
	@true

client/tools/objparser: client/tools/objparser.c
	@$(CC) $(OPTIMIZATION) $(CSTD) $< -o $@
	@echo "$(ANSI_BG_GREY)" "[CC] " "$(ANSI_RESET)" $@

client/src/tmp/objs.c: $(MESHASSETS)
client/src/tmp/objs.c: client/tools/objgen
	@mkdir -p client/src/tmp/
	@client/tools/objgen client/src/tmp/objs client/mesh/
	@echo "$(ANSI_GREY)" "[OBJ]" "$(ANSI_RESET)" $@
client/src/tmp/objs.h: client/src/tmp/objs.c
	@true

client/src/tmp/sfx.c: $(SFX_ASSETS)
client/src/tmp/sfx.c: client/tools/sfxgen
	@mkdir -p client/src/tmp/
	@client/tools/sfxgen client/src/tmp/sfx client/sfx/
	@echo "$(ANSI_GREY)" "[SFX]" "$(ANSI_RESET)" $@
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
runlld: all
	gdb ./wolkenwelten -ex "r $(TEST_WORLD) -soundVolume=10 -debugInfo=1 -serverName=localhost"
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

client/src/network/client.o: client/src/network/platform_specific/client_bsd.h
