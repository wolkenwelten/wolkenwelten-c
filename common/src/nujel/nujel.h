#pragma once
#include "../common.h"

typedef enum lType {
	ltNoAlloc = 0, ltNil, ltBool, ltList, ltLambda, ltInt, ltFloat, ltVec, ltString, ltCString, ltSymbol, ltNativeFunc, ltInf
} lType;

typedef struct lVal     lVal;
typedef struct lClosure lClosure;
typedef struct lString  lString;
typedef struct lCString lCString;

typedef struct {
	union {
		char c[16];
		u64 v[2];
	};
} lSymbol;

typedef struct {
	lVal *car,*cdr;
} lCell;

struct lVal {
	u8 flags;
	lType type;
	union {
		lVal         *vNA;
		bool          vBool;
		lCell         vList;
		int           vInt;
		vec           vVec;
		float         vFloat;
		lString      *vString;
		lCString     *vCString;
		lSymbol       vSymbol;
		lClosure     *vLambda;
		lVal       *(*vNativeFunc)(lClosure *, lVal *);
	};
};
#define lfMarked (1)

struct lClosure {
	lClosure *parent;
	lVal *data;
	lVal *text;
	unsigned int flags;
};

struct lString {
	char *buf,*data;
	union {
		char *bufEnd;
		lString *next;
	};
	int len;
};

struct lCString {
	const char *data;
	union {
		const char *bufEnd;
		lCString *next;
	};
};

#define foreach(n,v) for(lVal *n = v;(n != NULL) && (n->type == ltList); n = n->vList.cdr)

void      lInit             ();
int       lMemUsage         ();
void      lPrintError       (const char *format, ...);

lClosure *lClosureAlloc     ();
lClosure *lClosureNew       (lClosure *parent);
void      lClosureFree      (lClosure *c);
lVal     *lClosureAddNF     (lClosure *c, const char *sym, lVal *(*func)(lClosure *,lVal *));

lVal     *lValAlloc         ();
void      lValFree          (lVal *v);

lString  *lStringAlloc      ();
void      lStringFree       (lString *s);
lString  *lStringNew        (const char *str, uint len);

lCString *lCStringAlloc     ();
void      lCStringFree      (lCString *s);

void      lClosureGC        ();
lVal     *lParseSExprCS     (const char *str);
void      lPrintVal         (lVal *v);

lVal     *lValNativeFunc(lVal *(*func)(lClosure *,lVal *));
lVal     *lResolveNativeSymBuiltin(const lSymbol s);
lVal     *lResolveNativeSym (const lSymbol s);
lVal     *lResolveClosureSym(lClosure *c, const lSymbol s);
lVal     *lDefineClosureSym (lClosure *c, const lSymbol s);
lVal     *lResolveSym       (lClosure *c, const lSymbol s);
lVal     *lEval             (lClosure *c, lVal *v);
lType     lTypecast         (lVal *a, lVal *b);

lVal     *lValNil       ();
lVal     *lCons         (lVal *car,lVal *cdr);
lVal     *lValBool      (bool v);
lVal     *lValInf       ();
lVal     *lValInt       (int v);
lVal     *lValFloat     (float v);
lVal     *lValVec       (const vec v);
lVal     *lValSymS      (const lSymbol s);
lVal     *lValSym       (const char *s);
lVal     *lValString    (const char *s);
lVal     *lnfCat        (lClosure *c, lVal *v);
lVal     *lValCopy      (lVal *dst, const lVal *src);

static inline lVal *lValDup(const lVal *v){
	return v == NULL ? NULL : lValCopy(lValAlloc(),v);
}
static inline lVal *lWrap(lVal *v){
	return lCons(lValSym("repldo"),lCons(lCons(NULL,NULL),v));
}
