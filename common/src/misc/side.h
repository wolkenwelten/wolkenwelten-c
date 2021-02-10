#pragma once

typedef enum {
	sideFront,
	sideBack,
	sideTop,
	sideBottom,
	sideLeft,
	sideRight,
	sideMAX
} side;

typedef enum {
	sideMaskFront = 1 << sideFront,
	sideMaskBack = 1 << sideBack,
	sideMaskTop = 1 << sideTop,
	sideMaskBottom = 1 << sideBottom,
	sideMaskLeft = 1 << sideLeft,
	sideMaskRight = 1 << sideRight,
	sideMaskALL = (1 << sideMAX) - 1
} sideMask;
