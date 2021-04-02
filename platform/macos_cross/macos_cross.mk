AS                   := as

OSX_CLIENT_LIBS      := -F ~/ReleaseFrameworks/ -framework SDL2 -framework SDL2_mixer -framework OpenGL
OSX_CLIENT_CINCLUDES := -I ~/ReleaseFrameworks/include/SDL2

OSX_APP              := releases/macos/wolkenwelten-$(VERSION_NAME).app

.PHONY: release.macos
release: release.macos

release.macos: releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Info.plist
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/MacOS/wolkenwelten
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/MacOS/wolkenwelten-server
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Frameworks/SDL2.framework
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Frameworks/SDL2_mixer.framework
releases/macos/wolkenwelten-macos-$(VERSION_NAME).tgz: $(OSX_APP)/Contents/Resources/wolkenwelten.icns
	cd releases/macos/ && tar -czf wolkenwelten-macos-$(VERSION_NAME).tgz wolkenwelten-$(VERSION_NAME).app

$(OSX_APP)/Contents/MacOS/wolkenwelten: LIBS      += $(OSX_CLIENT_LIBS)
$(OSX_APP)/Contents/MacOS/wolkenwelten: CFLAGS    += $(OSX_CLIENT_CFLAGS)
$(OSX_APP)/Contents/MacOS/wolkenwelten: CINCLUDES += $(OSX_CLIENT_CINCLUDES)
$(OSX_APP)/Contents/MacOS/wolkenwelten: $(CLIENT_SRCS) $(CLIENT_HDRS)
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/assets.c client/src/tmp/assets.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/meshassets.c client/src/tmp/meshassets.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/objs.c client/src/tmp/objs.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: client/src/tmp/sfx.c client/src/tmp/sfx.h
$(OSX_APP)/Contents/MacOS/wolkenwelten: common/src/tmp/cto.c common/src/tmp/cto.h
	mkdir -p $(OSX_APP)/Contents/MacOS
	o64-clang $(CLIENT_SRCS)  -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CINCLUDES) $(WARNINGS) $(LIBS)
	x86_64-apple-darwin16-strip -SxX $@
	x86_64-apple-darwin16-install_name_tool -change "@rpath/SDL2.framework/Versions/A/SDL2" "@executable_path/../Frameworks/SDL2.framework/SDL2" $@
	x86_64-apple-darwin16-install_name_tool -change "@rpath/SDL2_mixer.framework/Versions/A/SDL2_mixer" "@executable_path/../Frameworks/SDL2_mixer.framework/SDL2_mixer" $@

$(OSX_APP)/Contents/MacOS/wolkenwelten-server: CFLAGS    += $(SERVER_CFLAGS)
$(OSX_APP)/Contents/MacOS/wolkenwelten-server: CINCLUDES += $(SERVER_CINCLUDES)
$(OSX_APP)/Contents/MacOS/wolkenwelten-server: LIBS      += $(SERVER_LIBS)
$(OSX_APP)/Contents/MacOS/wolkenwelten-server: $(SERVER_SRCS) $(SERVER_HDRS)
$(OSX_APP)/Contents/MacOS/wolkenwelten-server: server/src/tmp/sfx.c server/src/tmp/sfx.h
$(OSX_APP)/Contents/MacOS/wolkenwelten-server: server/src/tmp/objs.c server/src/tmp/objs.h
$(OSX_APP)/Contents/MacOS/wolkenwelten-server: common/src/tmp/cto.c common/src/tmp/cto.h
	mkdir -p $(OSX_APP)/Contents/MacOS
	o64-clang $(SERVER_SRCS) -o $@ $(RELEASE_OPTIMIZATION) $(CFLAGS) $(CINCLUDES) $(WARNINGS) $(LIBS)
	x86_64-apple-darwin16-strip -SxX $@

$(OSX_APP)/Contents/Info.plist:  platform/macos/Info.plist
	mkdir -p $(OSX_APP)/Contents
	cp -f platform/macos/Info.plist $(OSX_APP)/Contents/Info.plist

$(OSX_APP)/Contents/Frameworks/SDL2.framework: ~/ReleaseFrameworks/SDL2.framework
	mkdir -p $(OSX_APP)/Contents/Frameworks/
	rsync -a $< $(OSX_APP)/Contents/Frameworks/

$(OSX_APP)/Contents/Frameworks/SDL2_mixer.framework: ~/ReleaseFrameworks/SDL2_mixer.framework
	mkdir -p $(OSX_APP)/Contents/Frameworks/
	rsync -a $< $(OSX_APP)/Contents/Frameworks/

$(OSX_APP)/Contents/Resources/wolkenwelten.icns: platform/macos_cross/wolkenwelten.icns
	mkdir -p $(OSX_APP)/Contents/Resources/
	cp -f "$<" "$@"
