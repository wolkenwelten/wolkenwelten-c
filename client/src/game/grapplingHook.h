#pragma once
#include "../../../common/src/common.h"

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
