#pragma once
#include "../common.h"

extern sfx *sfxBomb;
extern sfx *sfxChomp;
extern sfx *sfxFalling;
extern sfx *sfxHoho;
extern sfx *sfxHoo;
extern sfx *sfxHookFire;
extern sfx *sfxHookHit;
extern sfx *sfxHookReturned;
extern sfx *sfxHookRope;
extern sfx *sfxImpact;
extern sfx *sfxJet;
extern sfx *sfxNibble;
extern sfx *sfxPhaser;
extern sfx *sfxPock;
extern sfx *sfxProjectile;
extern sfx *sfxStep;
extern sfx *sfxStomp;
extern sfx *sfxTock;
extern sfx *sfxUngh;
extern sfx *sfxWind;
extern sfx *sfxYahoo;

void sfxPlay   (sfx *b, float volume);
void sfxLoop   (sfx *b, float volume);
void sfxPlayPos(sfx *b, float volume, const vec pos);
