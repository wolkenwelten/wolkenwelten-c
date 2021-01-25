#pragma once
#include "../common.h"

typedef enum lType {
	ltNoAlloc = 0, ltBool, ltPair, ltLambda, ltInt, ltFloat, ltVec, ltString, ltCString, ltSymbol, ltNativeFunc, ltInf, ltArray
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
} lPair;

typedef struct {
	lVal **data;
	int length;
} lArray;

struct lVal {
	u8 flags;
	lType type;
	union {
		lVal         *vNA;
		bool          vBool;
		lPair         vList;
		int           vInt;
		vec           vVec;
		float         vFloat;
		lString      *vString;
		lCString     *vCString;
		lSymbol       vSymbol;
		lClosure     *vLambda;
		lArray        vArr;
		lVal       *(*vNativeFunc)(lClosure *, lVal *);
	};
};

struct lClosure {
	lClosure *parent;
	lVal *data;
	lVal *text;
	uint flags;
};
#define lfMarked  (1)
#define lfDynamic (2)
#define lfNoGC    (4)

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

extern lSymbol symQuote,symArr;

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
void      lDisplayVal       (lVal *v);
void      lWriteVal         (lVal *v);

lVal     *lValNativeFunc    (lVal *(*func)(lClosure *,lVal *));
lVal     *lResolveNativeSymBuiltin(const lSymbol s);
lVal     *lResolveNativeSym (const lSymbol s);
lVal     *lGetClosureSym    (lClosure *c, const lSymbol s);
lVal     *lResolveClosureSym(lClosure *c, const lSymbol s);
lVal     *lDefineClosureSym (lClosure *c, const lSymbol s);
lVal     *lResolveSym       (lClosure *c, const lSymbol s);
lVal     *lApply            (lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *));
lVal     *lCast             (lClosure *c, lVal *v, lType t);
lVal     *lEval             (lClosure *c, lVal *v);
lType     lTypecast         (const lType a,const lType b);
lType     lTypecastList     (lVal *a);

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

#define forEach(n,v) for(lVal *n = v;(n != NULL) && (n->type == ltPair) && (n->vList.car != NULL); n = n->vList.cdr)

static inline lVal *lValDup(const lVal *v){
	return v == NULL ? NULL : lValCopy(lValAlloc(),v);
}
static inline lVal *lWrap(lVal *v){
	return lCons(lValSym("begin"),lCons(lCons(NULL,NULL),v));
}
static inline lVal *lEvalCast(lClosure *c, lVal *v){
	lVal *t = lApply(c,v,lEval);
	return lCast(c,t,lTypecastList(t));
}
static inline lVal *lEvalCastSpecific(lClosure *c, lVal *v, const lType type){
	lVal *t = lApply(c,v,lEval);
	return lCast(c,t,type);
}
static inline lVal *lEvalCastNumeric(lClosure *c, lVal *v){
	lVal *t = lApply(c,v,lEval);
	lType type = lTypecastList(t);
	if(type == ltString){type = ltFloat;}
	return lCast(c,t,type);
}
static inline lVal *lLastCar(lVal *v){
	forEach(a,v){if(a->vList.cdr == NULL){return a->vList.car;}}
	return NULL;
}
static inline lVal *lCarOrV(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.car : v;
}
static inline lVal *lCarOrN(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.car : NULL;
}
static inline lVal *lCadrOrN(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? lCarOrN(v->vList.cdr) : NULL;
}
static inline int lListLength(lVal *v){
	int i = 0;
	for(lVal *n = v;(n != NULL) && (n->type == ltPair) && (n->vList.car != NULL); n = n->vList.cdr){i++;}
	return i;
}

#define lEvalCastApply(FUNC, c , v) do { \
	lVal *t = lEvalCast(c,v); \
	if((t == NULL) || (t->type != ltPair)){return lValInt(0);} \
	lVal *d = lValDup(t->vList.car); \
	if(d == NULL){return lValInt(0);} \
	switch(d->type){ \
	default:      return lValInt(0); \
	case ltInf:   return lValInf(); \
	case ltInt:   return FUNC##I(d,t); \
	case ltFloat: return FUNC##F(d,t); \
	case ltVec:   return FUNC##V(d,t); \
	}} while (0)
