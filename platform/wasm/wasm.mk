EMCC         := emcc
EMLIBS       := -s USE_SDL=2 -s USE_SDL_MIXER=2
CLIENT_EMMEM := -s TOTAL_MEMORY=768MB
SERVER_EMMEM := -s TOTAL_MEMORY=1536MB

CLIENT_WASM_OBJS := $(CLIENT_SRCS:.c=.wo) $(CLIENT_TMP_SRCS:.c=.wo)
SERVER_WASM_OBJS := $(SERVER_SRCS:.c=.wo) $(SERVER_TMP_SRCS:.c=.wo)

$(CLIENT_WASM_OBJS): | client/src/tmp/sfx.h
$(CLIENT_WASM_OBJS): | client/src/tmp/objs.h
$(CLIENT_WASM_OBJS): | common/src/tmp/cto.c
$(CLIENT_WASM_OBJS): | common/src/tmp/assets.h

$(SERVER_WASM_OBJS): | server/src/tmp/assets.h
$(SERVER_WASM_OBJS): | server/src/tmp/objs.h
$(SERVER_WASM_OBJS): | server/src/tmp/sfx.h
$(SERVER_WASM_OBJS): | common/src/tmp/cto.c
$(SERVER_WASM_OBJS): | common/src/tmp/assets.h

%.wo: %.c
	$(EMCC) -c $< $(EMLIBS) -D_GNU_SOURCE $(CSTD) -O3 -fno-rtti -o $@ -MMD

all: wolkenwelten

.PHONY: release.wasm
release: release.wasm

release: release.wasm
release.wasm: releases/wasm/index.html
release.wasm: releases/wasm/server.js
release.wasm: releases/wasm/manifest.json

.PHONY: common/nujel/nujel.wa
common/nujel/nujel.wa:
	$(MAKE) -C common/nujel nujel.wa

releases/wasm/index.html: $(CLIENT_WASM_OBJS) $(COMMON_WASM_OBJS) common/nujel/nujel.wa
	@mkdir -p releases/wasm/
	$(EMCC) $^ -D_GNU_SOURCE $(CSTD) -O3 -fno-rtti --closure 0 -s MINIFY_HTML=0 -s OFFSCREEN_FRAMEBUFFER=1 -s OFFSCREENCANVAS_SUPPORT=1 -s MAX_WEBGL_VERSION=2 $(CLIENT_EMMEM) $(EMLIBS) --shell-file platform/wasm/shell.html -o $@

releases/wasm/server.js: $(SERVER_WASM_OBJS) $(COMMON_WASM_OBJS) $(NUJEL_WASM_OBJS) common/nujel/nujel.wa
	@mkdir -p releases/wasm/
	$(EMCC) $^ -D_GNU_SOURCE $(CSTD) -O3 -s BUILD_AS_WORKER=1 -s EXPORTED_FUNCTIONS="['_wasmInit','_wasmTranceive']" -fno-rtti --closure 0 $(SERVER_EMMEM) -o releases/wasm/server.js

releases/wasm/manifest.json: platform/wasm/manifest.json
	@mkdir -p releases/wasm/
	cp -f $< $@

client/src/network/client.wo: client/src/network/platform_specific/client_wasm.h
