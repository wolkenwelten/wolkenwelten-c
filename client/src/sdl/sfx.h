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

extern sfx *sfxFalling;
extern sfx *sfxHoho;
extern sfx *sfxHoo;
extern sfx *sfxImpact;
extern sfx *sfxPhaser;
extern sfx *sfxBomb;
extern sfx *sfxTock;
extern sfx *sfxPock;
extern sfx *sfxStomp;
extern sfx *sfxStep;
extern sfx *sfxUngh;
extern sfx *sfxYahoo;
extern sfx *sfxHookFire;
extern sfx *sfxHookHit;
extern sfx *sfxHookReturned;
extern sfx *atmosfxHookRope;
extern sfx *atmosfxWind;

sfx *sfxNew(unsigned char *data,size_t dataLen);
void sfxFree(sfx *b);
void sfxPlay(sfx *b, float volume);
void sfxLoop(sfx *b, float volume);

bgm *bgmNew(unsigned char *data,size_t dataLen);
void bgmFree(bgm *b);
void bgmPlay(bgm *b,float volume);
