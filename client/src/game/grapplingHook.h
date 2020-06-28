#pragma once
#include "../game/character.h"
#include "../game/entity.h"
#include "../gfx/mesh.h"

struct grapplingHook {
	entity       *ent;
	mesh        *rope;
	character *parent;
	float  goalLength;
	bool       hooked;
	bool    returning;
};
typedef struct grapplingHook grapplingHook;

grapplingHook *grapplingHookNew   (character *shooter );
void  grapplingHookFree           (grapplingHook *g);
bool  grapplingHookGetHooked      (grapplingHook *g);
float grapplingHookGetLength      (grapplingHook *g);
float grapplingHookGetGoalLength  (grapplingHook *g);
void  grapplingHookSetGoalLength  (grapplingHook *g, float gl);
void  grapplingHookReturnHook     (grapplingHook *g);
bool  grapplingHookReturnToParent (grapplingHook *g, float speed);
void  grapplingHookPullTowards    (grapplingHook *g, character *pull);
bool  grapplingHookUpdate         (grapplingHook *g);
void  grapplingHookDrawRopes();
