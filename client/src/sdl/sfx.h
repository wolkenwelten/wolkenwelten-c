#pragma once

#ifdef EMSCRIPTEN
	#include <SDL2/SDL_mixer.h>
#else
	#include <SDL_mixer.h>
#endif
#include "../../../common/src/common.h"

extern int sfxEnable;
void sfxInit();
void sfxFreeAll();

struct bgm {
	Mix_Music *mixMusic;
	int chan;
};

struct sfx {
	Mix_Chunk *mixChunk;
	int chan;
};

#include "../tmp/sfx.h"

sfx *sfxNew (const void *data,size_t dataLen);
void sfxFree(sfx *b);
void sfxPlay(sfx *b, float volume);
void sfxLoop(sfx *b, float volume);

bgm *bgmNew (const void *data,size_t dataLen);
void bgmFree(bgm *b);
void bgmPlay(bgm *b,float volume);

void sfxStopAll();
