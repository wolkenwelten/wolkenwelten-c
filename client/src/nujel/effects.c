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
#include "effects.h"
#include "../gfx/effects.h"

static lVal *lEffectExplodeBomb(lClosure *c, lVal *v){
	const vec pos = requireVec(c, lCar(v));
	const float pow = requireFloat(c, lCadr(v));
	fxExplosionBomb(pos, pow);
	return lCar(v);
}

void lOperatorsEffects(lClosure *c){
	lAddNativeFunc(c,"effect/explode/bomb", "[pos power]", "Create a new explosion at POS with POWER", lEffectExplodeBomb);
}
