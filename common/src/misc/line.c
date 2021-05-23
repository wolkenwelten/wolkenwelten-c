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

#include "line.h"
#include <stdlib.h>
#include <math.h>

void lineFromTo(int x0, int y0, int z0, int x1, int y1, int z1, void (*func)(int x, int y, int z)){
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int dz = abs(z1 - z0);

	int dmax = MAX(dx,MAX(dy,dz));

	int ex = 2 * dx - dmax;
	int ey = 2 * dy - dmax;
	int ez = 2 * dx - dmax;

	int dirx = x0 < x1 ? 1 : -1;
	int diry = y0 < y1 ? 1 : -1;
	int dirz = z0 < z1 ? 1 : -1;

	int x = x0;
	int y = y0;
	int z = z0;
	for(int i=0;i<dmax;i++){
		if(ex > dmax){
			x += dirx;
			ex -= dmax * 2;
		}
		ex += dx * 2;
		if(ey > dmax){
			y += diry;
			ey -= dmax * 2;
		}
		ey += dy * 2;
		if(ez > dmax){
			z += dirz;
			ez -= dmax * 2;
		}
		ez += dz * 2;
		func(x,y,z);
	}
	func(x,y,z);
}
