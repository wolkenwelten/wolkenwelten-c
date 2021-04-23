EMCC         := emcc
EMLIBS       := -s USE_SDL=2 -s USE_SDL_MIXER=2 -s GL_UNSAFE_OPTS=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2
CLIENT_EMMEM := -s TOTAL_MEMORY=768MB
SERVER_EMMEM := -s TOTAL_MEMORY=1536MB

CLIENT_WASM_OBJS := $(CLIENT_SRCS:.c=.wo)
SERVER_WASM_OBJS := $(SERVER_SRCS:.c=.wo)

%.wo: %.c
	$(EMCC) -c $< $(EMLIBS) -D_GNU_SOURCE -std=c99 -O3 -fno-rtti -s WASM=1 -o $@ -MMD

all: wolkenwelten

.PHONY: release.wasm
release: release.wasm

release: release.wasm
release.wasm: releases/wasm/wolkenwelten.html
release.wasm: releases/wasm/server.js
release.wasm: releases/wasm/manifest.json

releases/wasm/wolkenwelten.html: $(CLIENT_WASM_OBJS)
	@mkdir -p releases/wasm/
	$(EMCC) $^ -D_GNU_SOURCE -std=c99 -O3 -fno-rtti --closure 0 -s MINIFY_HTML=0 -s WASM=1 -s OFFSCREEN_FRAMEBUFFER=1 -s OFFSCREENCANVAS_SUPPORT=1 $(CLIENT_EMMEM) $(EMLIBS) --shell-file platform/wasm/shell.html -o releases/wasm/index.html

releases/wasm/server.js: $(SERVER_WASM_OBJS)
	@mkdir -p releases/wasm/
	$(EMCC) $^ -D_GNU_SOURCE -std=c99 -O3 -s BUILD_AS_WORKER=1 -s EXPORTED_FUNCTIONS="['_wasmInit','_wasmTranceive']" -fno-rtti --closure 0 $(SERVER_EMMEM) -o releases/wasm/server.js

releases/wasm/manifest.json: platform/wasm/manifest.json
	@mkdir -p releases/wasm/
	cp -f $< $@
