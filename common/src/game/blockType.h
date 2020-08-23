#pragma once
#include "../../../common/src/common.h"

void          blockTypeInit();

const char   *blockTypeGetName         (uint8_t b);
int           blockTypeGetHP           (uint8_t b);
blockCategory blockTypeGetCat          (uint8_t b);
bool          blockTypeValid           (uint8_t b);
uint16_t      blockTypeGetTexX         (uint8_t b, int side);
uint16_t      blockTypeGetTexY         (uint8_t b, int side);
uint32_t      blockTypeGetParticleColor(uint8_t b);
mesh         *blockTypeGetMesh         (uint8_t b);
