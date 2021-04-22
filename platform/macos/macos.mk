AS               := as
CC               := clang

CLIENT_LIBS      := -F /Library/Frameworks -framework SDL2 -framework SDL2_mixer -framework OpenGL
CLIENT_CFLAGS    := -I /Library/Frameworks/SDL2.framework/Headers -I /Library/Frameworks/SDL2_mixer.framework/Headers -DGL_SILENCE_DEPRECATION

OSX_APP          := releases/macos/wolkenwelten-$(VERSION_NAME).app

.PHONY: all
all: wolkenwelten wolkenwelten-server

.PHONY: release.macos
release: release.macos

# Assemble empty object files, easier than removing the deps which wont assemble due to
# clang as missing --defsym support
%.o: %.s
	$(AS) -c -o $@ -- < /dev/null

release.macos: releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Info.plist
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/MacOS/wolkenwelten
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/MacOS/wolkenwelten-server
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Frameworks/SDL2.framework
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Frameworks/SDL2_mixer.framework
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Resources/wolkenwelten.icns
	cd releases/macos/ && tar -czf wolkenwelten-macos-$(VERSION_NAME).tgz wolkenwelten-$(VERSION_NAME).app

$(OSX_APP)/Contents/MacOS/wolkenwelten: CFLAGS    += $(CLIENT_CFLAGS)
$(OSX_APP)/Contents/MacOS/wolkenwelten: CINCLUDES += $(CLIENT_CINCLUDES)
$(OSX_APP)/Contents/MacOS/wolkenwelten: LIBS      += $(CLIENT_LIBS)
$(OSX_APP)/Contents/MacOS/wolkenwelten: $(CLIENT_SRCS) $(CLIENT_HDRS)
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/gfxAssets.c client/src/tmp/gfxAssets.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/sfxAssets.c client/src/tmp/sfxAssets.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/shdAssets.c client/src/tmp/shdAssets.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/txtAssets.c client/src/tmp/txtAssets.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/nujAssets.c client/src/tmp/nujAssets.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/meshAssets.c client/src/tmp/meshAssets.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/objs.c client/src/tmp/objs.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/sfx.c client/src/tmp/sfx.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: common/src/tmp/cto.c
	mkdir -p $(OSX_APP)/Contents/MacOS
	$(CC) $(CLIENT_SRCS) -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CINCLUDES) $(WARNINGS) $(LIBS)
	strip -SxX $@
	install_name_tool -change "@rpath/SDL2.framework/Versions/A/SDL2" "@executable_path/../Frameworks/SDL2.framework/SDL2" $@
	install_name_tool -change "@rpath/SDL2_mixer.framework/Versions/A/SDL2_mixer" "@executable_path/../Frameworks/SDL2_mixer.framework/SDL2_mixer" $@

$(OSX_APP)/Contents/MacOS/wolkenwelten-server: $(SERVER_SRCS) $(SERVER_HDRS)
$(OSX_APP)/Contents/MacOS/wolkenwelten-server: server/src/tmp/sfx.c server/src/tmp/sfx.h
$(OSX_APP)/Contents/MacOS/wolkenwelten-server: server/src/tmp/objs.c server/src/tmp/objs.h
$(OSX_APP)/Contents/MacOS/wolkenwelten-server: common/src/tmp/cto.c
	mkdir -p $(OSX_APP)/Contents/MacOS
	$(CC) $(SERVER_SRCS) -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CINCLUDES) $(WARNINGS) $(LIBS)
	strip -SxX $@

$(OSX_APP)/Contents/Info.plist:  platform/macos/Info.plist
	mkdir -p $(OSX_APP)/Contents
	cp -f platform/macos/Info.plist $(OSX_APP)/Contents/Info.plist

$(OSX_APP)/Contents/Frameworks/SDL2.framework: ~/ReleaseFrameworks/SDL2.framework
	mkdir -p $(OSX_APP)/Contents/Frameworks/
	rsync -a $< $(OSX_APP)/Contents/Frameworks/

$(OSX_APP)/Contents/Frameworks/SDL2_mixer.framework: ~/ReleaseFrameworks/SDL2_mixer.framework
	mkdir -p $(OSX_APP)/Contents/Frameworks/
	rsync -a $< $(OSX_APP)/Contents/Frameworks/

$(OSX_APP)/Contents/Resources/wolkenwelten.icns: releases/macos/wolkenwelten.iconset/icon_16x16.png
$(OSX_APP)/Contents/Resources/wolkenwelten.icns: releases/macos/wolkenwelten.iconset/icon_16x16@2x.png
$(OSX_APP)/Contents/Resources/wolkenwelten.icns: releases/macos/wolkenwelten.iconset/icon_32x32.png
$(OSX_APP)/Contents/Resources/wolkenwelten.icns: releases/macos/wolkenwelten.iconset/icon_32x32@2x.png
$(OSX_APP)/Contents/Resources/wolkenwelten.icns: releases/macos/wolkenwelten.iconset/icon_64x64.png
$(OSX_APP)/Contents/Resources/wolkenwelten.icns: releases/macos/wolkenwelten.iconset/icon_64x64@2x.png
$(OSX_APP)/Contents/Resources/wolkenwelten.icns: releases/macos/wolkenwelten.iconset/icon_128x128.png
$(OSX_APP)/Contents/Resources/wolkenwelten.icns: releases/macos/wolkenwelten.iconset/icon_128x128@2x.png
$(OSX_APP)/Contents/Resources/wolkenwelten.icns: releases/macos/wolkenwelten.iconset/icon_256x256.png
	mkdir -p $(OSX_APP)/Contents/Resources/
	iconutil -c icns -o $@ releases/macos/wolkenwelten.iconset

releases/macos/wolkenwelten.iconset/icon_16x16.png: web/favicon16.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "16x16" $< $@
releases/macos/wolkenwelten.iconset/icon_32x32.png: web/favicon32.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "32x32" $< $@
releases/macos/wolkenwelten.iconset/icon_16x16@2x.png: web/favicon32.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "32x32" $< $@
releases/macos/wolkenwelten.iconset/icon_32x32@2x.png: web/favicon.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "64x64" $< $@
releases/macos/wolkenwelten.iconset/icon_64x64.png: web/favicon.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "64x64" $< $@
releases/macos/wolkenwelten.iconset/icon_64x64@2x.png: web/favicon.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "128x128" $< $@
releases/macos/wolkenwelten.iconset/icon_128x128.png: web/favicon.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "128x128" $< $@
releases/macos/wolkenwelten.iconset/icon_128x128@2x.png: web/favicon.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "256x256" $< $@
releases/macos/wolkenwelten.iconset/icon_256x256.png: web/favicon.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "256x256" $< $@
