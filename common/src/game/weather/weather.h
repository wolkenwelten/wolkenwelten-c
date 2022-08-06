#pragma once
#include "../../../../common/src/common.h"

#include "clouds.h"
#include "rain.h"
#include "snow.h"
#include "storm.h"
#include "wind.h"

void tryLightning();
void lightningDrawOverlay();

void weatherInit();
void weatherUpdateAll();

void weatherSendUpdate(uint c);
