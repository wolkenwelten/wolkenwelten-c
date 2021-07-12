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

struct sfx {
	Mix_Chunk *mixChunk;
	int chan;
};

#include "../tmp/sfx.h"

extern sfx sfxList[32];

sfx *sfxNew    (const void *data, unsigned int dataLen, const char *lName);
void sfxFree   (sfx *b);
void sfxPlay   (sfx *b, float volume);
void sfxLoop   (sfx *b, float volume);
void sfxPlayPos(sfx *b, float volume, const vec pos);

void sfxStopAll();
