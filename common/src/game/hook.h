#pragma once
#include "../../../common/src/common.h"

void fxBlockBreak        (const vec pos, uchar b);

hook *hookNew            (character *shooter);
void  hookFree           (      hook *g);
bool  hookGetHooked      (const hook *g);
float hookGetLength      (const hook *g);
float hookGetRopeLength  (const hook *g);
float hookGetGoalLength  (const hook *g);
void  hookSetGoalLength  (      hook *g, float gl);
void  hookReturnHook     (      hook *g);
bool  hookReturnToParent (      hook *g, float speed);
bool  hookUpdate         (      hook *g);

hook *hookGetByBeing(being b);
being hookGetBeing  (const hook *h);
