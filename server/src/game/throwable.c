#include "throwable.h"

#include "entity.h"

void throwableNew(const vec pos, const vec rot, float speed, const item itm, being thrower, u16 flags){
	throwable *a = throwableAlloc();
	a->ent       = entityNew(pos,rot);
	a->ent->vel  = vecMulS(vecDegToVec(rot),speed);
	a->itm       = itm;
	a->flags     = flags;
	a->counter   = 0;
	a->nextFree  = -1;
	a->thrower   = thrower;
	throwableSendUpdate(-1,a - throwableList);
}
