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
#include "time.h"

#include "boolean.h"
#include "casting.h"

#include <time.h>
#include <sys/time.h>

static u64 getMSecs(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
}


lVal *lnfTime(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(time(NULL));
}

lVal *lnfTimeMsecs(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValInt(getMSecs());
}

lVal *lnfStrftime(lClosure *c, lVal *v){
	int timestamp = 0;
	const char *format = "%Y-%m-%d %H:%M:%S";

	v = getLArgI(c,v,&timestamp);
	v = getLArgS(c,v,&format);

	char buf[1024];
	time_t ts = timestamp;
	struct tm *info = localtime(&ts);
	strftime(buf,sizeof(buf),format,info);

	return lValString(buf);
}

void lAddTimeFuncs(lClosure *c){
	lAddNativeFunc(c,"time",             "[]",         "Returns unix time",lnfTime);
	lAddNativeFunc(c,"strftime",         "[ts format]","Returns TS as a date using FORMAT (uses strftime)",lnfStrftime);
	lAddNativeFunc(c,"time-milliseconds","[]",         "Returns monotonic msecs",lnfTimeMsecs);
}
