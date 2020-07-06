#pragma once
#include "../network/packet.h"
#include <stdint.h>

void msgSendPlayerPos();
void msgRequestPlayerSpawnPos();
void msgNewGrenade(int ID, float x, float y, float z, float yaw, float pitch, float roll, float pwr);
void msgBeamBlast(int ID, float x, float y, float z, float yaw, float pitch, float roll, float pwr);
void msgItemDropPickup(int i);
void msgItemDropNew(float x, float y, float z, float vx, float vy, float vz, int ID, int amount);
void msgRequestChungus(int x, int y, int z);
void msgParseGetChunk(packetHuge *p);
void msgPlaceBlock(int x, int y, int z, uint8_t b);
void msgMineBlock (int x, int y, int z, uint8_t b);
void msgGoodbye();