#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PI    (3.1415926535897932384626433832795f)
#define PI180 (3.1415926535897932384626433832795f / 180.f)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

struct sfx;
typedef struct sfx sfx;
struct bgm;
typedef struct bgm bgm;

#include "network/packet.h"
#include "gfx_structs.h"
#include "game_structs.h"
#include "misc/vec.h"

extern blockType blocks[256];
