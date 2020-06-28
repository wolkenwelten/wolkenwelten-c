#pragma once
#include "../gfx/mesh.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum blockCategory {
	NONE,
	DIRT,
	STONE,
	WOOD,
	LEAVES
} blockCategory;

void          blockTypeInit();
void          blockTypeGenMeshes();


void          blockTypeSetTex          (uint8_t b, int side, uint32_t tex);
const char   *blockTypeGetName         (uint8_t b);
int           blockTypeGetHP           (uint8_t b);
blockCategory blockTypeGetCat          (uint8_t b);
bool          blockTypeValid           (uint8_t b);
uint16_t      blockTypeGetTexX         (uint8_t b, int side);
uint16_t      blockTypeGetTexY         (uint8_t b, int side);
uint32_t      blockTypeGetParticleColor(uint8_t b);
mesh         *blockTypeGetMesh         (uint8_t b);

void blockTypeAddToMesh (uint8_t b,  mesh *m,float x,float y,float z, float w,float h,float d);

