#pragma once

#include "stdint.h"
#include "network/packet.h"
#include "gfx_structs.h"
#include "game_structs.h"
#include "misc/rng.h"
#include "misc/vec.h"

extern blockType blocks[256];
extern bool isClient;
extern int playerID;

extern char *ansiRS;
extern char *ansiFG[16];
extern  int msPerTick;

const char *lispEval        (const char *expr, bool humanReadable);
void        lispDefineInt   (const char *symbol, int val);
void        lispDefineString(const char *symbol, char *str);
void        lispDefineID    (const char *prefix, const char *symbol, int val);
