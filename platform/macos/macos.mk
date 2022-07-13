CC               := clang

NASMFLAGS        := -f macho64 --prefix "_"

CLIENT_LIBS      := -F /Library/Frameworks -framework SDL2 -framework SDL2_mixer -framework OpenGL
CLIENT_CFLAGS    := -I /Library/Frameworks/SDL2.framework/Headers -I /Library/Frameworks/SDL2_mixer.framework/Headers -DGL_SILENCE_DEPRECATION

OSX_APP          := releases/macos/wolkenwelten-$(VERSION_NAME).app

.PHONY: release.macos
release: release.macos

# Assemble empty object files, easier than removing the deps which wont assemble due to
# clang as missing --defsym support
%.o: %.s
	@$(AS) -c -o $@ -- < /dev/null
	@echo "$(ANSI_GREY)" "[00] " "$(ANSI_RESET)" $@

release.macos: releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Info.plist
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/MacOS/wolkenwelten
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/MacOS/wolkenwelten-server
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Frameworks/SDL2.framework
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Frameworks/SDL2_mixer.framework
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Resources/wolkenwelten.icns
	cd releases/macos/ && tar -czf wolkenwelten-macos-$(VERSION_NAME).tgz wolkenwelten-$(VERSION_NAME).app

$(OSX_APP)/Contents/MacOS/wolkenwelten: $(CLIENT_SRCS) $(CLIENT_TMP_SRCS) $(ASM_OBJS) common/nujel/nujel.a
	mkdir -p $(OSX_APP)/Contents/MacOS
	$(CC) $^ -o $@ $(RELEASE_OPTIMIZATION) $(CLIENT_CFLAGS) $(CFLAGS) $(CLIENT_CINCLUDES) $(CINCLUDES) $(WARNINGS) $(CLIENT_LIBS) $(LIBS)
	strip -SxX $@
	install_name_tool -change "@rpath/SDL2.framework/Versions/A/SDL2" "@executable_path/../Frameworks/SDL2.framework/SDL2" $@
	install_name_tool -change "@rpath/SDL2_mixer.framework/Versions/A/SDL2_mixer" "@executable_path/../Frameworks/SDL2_mixer.framework/SDL2_mixer" $@

$(OSX_APP)/Contents/MacOS/wolkenwelten-server: $(SERVER_SRCS) $(SERVER_TMP_SRCS) $(ASM_OBJS) common/nujel/nujel.a
	mkdir -p $(OSX_APP)/Contents/MacOS
	$(CC) $^ -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(SERVER_CFLAGS) $(CSTD) $(CINCLUDES) $(SERVER_CINCLUDES) $(LIBS) $(SERVER_LIBS)
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

releases/macos/wolkenwelten.iconset/icon_16x16.png: logo.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "16x16" $< $@
releases/macos/wolkenwelten.iconset/icon_32x32.png: logo.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "32x32" $< $@
releases/macos/wolkenwelten.iconset/icon_16x16@2x.png: logo.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "32x32" $< $@
releases/macos/wolkenwelten.iconset/icon_32x32@2x.png: logo.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "64x64" $< $@
releases/macos/wolkenwelten.iconset/icon_64x64.png: logo.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "64x64" $< $@
releases/macos/wolkenwelten.iconset/icon_64x64@2x.png: logo.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "128x128" $< $@
releases/macos/wolkenwelten.iconset/icon_128x128.png: logo.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "128x128" $< $@
releases/macos/wolkenwelten.iconset/icon_128x128@2x.png: logo.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "256x256" $< $@
releases/macos/wolkenwelten.iconset/icon_256x256.png: logo.png
	mkdir -p releases/macos/wolkenwelten.iconset/
	convert -resize "256x256" $< $@
