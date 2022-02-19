#pragma once

void lightSunlightAirISPC(u8 out[48][48][48], u8 curLight[16][16], const int x, const int y, const int z, const u8 sunlight);
void lightSunlightChunkISPC(u8 out[48][48][48], const u8 blockData[16][16][16], u8 curLight[16][16], const u8 blockLight[256], const int x, const int y, const int z, const u8 sunlight);
void lightBlurISPC(u8 out[48][48][48]);
