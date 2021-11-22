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
#include "../../../../common/src/common.h"
#include "../widget.h"

#include <math.h>
#include "../../gfx/textMesh.h"
#include "../../sdl/sdl.h"

void widgetDrawBackground(const widget *wid, textMesh *m, box2D area){
	(void)wid;
	int o = area.h/2 + sinf(getTicks()*0.001f)*((float)(area.h/6));
	textMeshVGradient(m,area.x,area.y    ,area.w,o,0xFFFFBF83, 0xFFFF6825);
	textMeshVGradient(m,area.x,area.y+o,area.w,area.h-o,0xFFFF6825, 0xFFE82410);
}
