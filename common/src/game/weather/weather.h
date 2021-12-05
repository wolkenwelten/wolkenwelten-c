#pragma once
#include "../../../../common/src/common.h"

#include "clouds.h"
#include "rain.h"
#include "snow.h"
#include "wind.h"

void weatherInit();
void weatherUpdateAll();

void weatherRecvUpdate(const packet *p);
void weatherSendUpdate(uint c);
