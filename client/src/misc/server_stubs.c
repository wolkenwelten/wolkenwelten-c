/* This file exists so we do not get undefined references when we link in code
 * meant for the client side.
 */
#include "../../../common/src/common.h"

void explode(const vec pos, float pwr, int style){
	(void)pos;
	(void)pwr;
	(void)style;
}
