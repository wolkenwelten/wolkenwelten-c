/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "environment.h"
#include "sfx.h"
#include "../game/character.h"
#include "../../../common/src/game/weather/rain.h"

static void windSound(const character *c){
	float windVol = c == NULL ? 0.f : vecMag(c->vel);
	if(windVol < 0.01f){
		sfxLoop(sfxWind,0.f);
	}else{
		windVol = MIN((windVol - 0.01f),1.0);
		sfxLoop(sfxWind,windVol);
	}
}

static void grapplingHookRopeSound(const character *c){
	sfxLoop(sfxHookRope, (c == NULL) || (c->hook == NULL) || c->hook->hooked ? 0.f : 1.f);
}

static void rainSound(){
	sfxLoop(sfxRainloop, (rainIntensity == 0) ? 0.f : MIN(1.f,(rainIntensity / 8.f)));
}

void environmentSoundsUpdate(){
	windSound(player);
	grapplingHookRopeSound(player);
	rainSound();
}
