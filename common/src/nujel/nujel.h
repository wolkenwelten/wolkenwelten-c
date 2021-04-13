#pragma once
#include "../common.h"

extern int lGCRuns;

typedef enum lType {
	ltNoAlloc = 0, ltBool, ltPair, ltLambda, ltInt, ltFloat, ltVec, ltString, ltSymbol, ltNativeFunc, ltInf, ltArray
} lType;

typedef struct lVal     lVal;
typedef struct lClosure lClosure;
typedef struct lString  lString;
typedef struct lArray   lArray;
typedef struct lNFunc   lNFunc;
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
	vec v;
	u16 nextFree;
	u16 flags;
} lVec;

struct lArray {
	lVal **data;
	int length;
	u8 flags;
	u32 nextFree;
};

struct lNFunc {
	lVal *(*fp)(lClosure *, lVal *);
	lVal *doc;
	u16 flags;
	u16 nextFree;
};

struct lVal {
	u8 flags;
	u8 type;
	union {
		u32        vCdr;
		bool       vBool;
		lPair      vList;
		int        vInt;
		float      vFloat;
		lSymbol    vSymbol;
	};
};

struct lClosure {
	lVal *data;
	lVal *text;
	u16 parent;
	u16 nextFree;
	u16 flags;
	u16 refCount;
};
#define lfMarked    ( 1)
#define lfDynamic   ( 2)
#define lfNoGC      ( 4)
#define lfConst     ( 8)
#define lfHeapAlloc (16)
#define lfObject    (32)
#define lfUsed      (64)

struct lString {
	const char *buf,*data,*bufEnd;
	u16 nextFree;
	u16 flags;
};

extern lSymbol symQuote,symArr,symIf,symCond,symWhen,symUnless,symLet,symBegin;

#define VAL_MAX (1<<18)
#define CLO_MAX (1<<14)
#define STR_MAX (1<<14)
#define ARR_MAX (1<<12)
#define NFN_MAX (1<<10)
#define VEC_MAX (1<<12)
#define SYM_MAX (1<<12)

#define VAL_MASK ((VAL_MAX)-1)
#define CLO_MASK ((CLO_MAX)-1)
#define STR_MASK ((STR_MAX)-1)
#define ARR_MASK ((ARR_MAX)-1)
#define NFN_MASK ((NFN_MAX)-1)
#define VEC_MASK ((VEC_MAX)-1)
#define SYM_MASK ((SYM_MAX)-1)

extern lVal     lValList    [VAL_MAX];
extern lClosure lClosureList[CLO_MAX];
extern lString  lStringList [STR_MAX];
extern lArray   lArrayList  [ARR_MAX];
extern lNFunc   lNFuncList  [NFN_MAX];
extern lVec     lVecList    [VEC_MAX];
extern lSymbol  lSymbolList [SYM_MAX];


void      lInit             ();
int       lMemUsage         ();
void      lPrintError       (const char *format, ...);

uint      lClosureAlloc     ();
lClosure *lClosureNewRoot   ();
uint      lClosureNew       (uint parent);
void      lClosureFree      (uint c);
lVal     *lClosureAddNF     (uint c, const char *sym, lVal *(*func)(lClosure *,lVal *));

lVal     *lValAlloc         ();
void      lValFree          (lVal *v);

uint      lArrayAlloc       ();
void      lArrayFree        (uint v);

uint      lStringAlloc      ();
void      lStringFree       (uint v);
uint      lStringNew        (const char *str, uint len);

void      lClosureGC        ();
void      lDisplayVal       (lVal *v);
void      lDisplayErrorVal  (lVal *v);
void      lWriteVal         (lVal *v);

void      lDefineVal        (lClosure *c, const char *sym, lVal *val);
void      lAddNativeFunc    (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));
lVal     *lValNativeFunc    (lVal *(*func)(lClosure *,lVal *), lVal *args, lVal *docString);
lVal     *lGetClosureSym    (uint      c, const lSymbol s);
lVal     *lResolveClosureSym(uint      c, const lSymbol s);
lVal     *lDefineClosureSym (uint      c, const lSymbol s);
lVal     *lMatchClosureSym  (uint      c, lVal *v, const lSymbol s);
lVal     *lResolveSym       (uint      c, lVal *v);
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

lVal *getLArgB(lClosure *c, lVal *v, bool *res);
lVal *getLArgI(lClosure *c, lVal *v, int *res);
lVal *getLArgF(lClosure *c, lVal *v, float *res);
lVal *getLArgV(lClosure *c, lVal *v, vec *res);
lVal *getLArgS(lClosure *c, lVal *v, const char **res);

#define lVec(i)  lVecList[i & VEC_MASK]
#define lVecV(i) lVec(i).v
#define lVecFlags(i) lVec(i).flags

#define lStrNull(val) (((val->vCdr & STR_MASK) == 0) || (lStringList[val->vCdr & STR_MASK].data == NULL))
#define lStr(val) lStringList[val->vCdr & STR_MASK]
#define lStrData(val) lStr(val).data
#define lStrBuf(val) lStr(val).buf
#define lStrEnd(val) lStr(val).bufEnd
#define lStrFlags(val) lStr(val).flags

#define lArrNull(val) (((val->vCdr & ARR_MASK) == 0) || (lArrayList[val->vCdr & ARR_MASK].data == NULL))
#define lArr(val) lArrayList[val->vCdr & ARR_MASK]
#define lArrLength(val) lArr(val).length
#define lArrData(val) lArr(val).data

#define lClo(i) lClosureList[i & CLO_MASK]
#define lCloParent(i) lClosureList[i & CLO_MASK].parent
#define lCloData(i) lClosureList[i & CLO_MASK].data
#define lCloText(i) lClosureList[i & CLO_MASK].text

#define lNFN(i) lNFuncList[i & NFN_MASK]

static inline lVal *lValDup(const lVal *v){
	return v == NULL ? NULL : lValCopy(lValAlloc(),v);
}
static inline lVal *lConst(lVal *v){
	v->flags |= lfConst;
	return v;
}
lVal *lnfBegin(lClosure *c, lVal *v);
static inline lVal *lWrap(lVal *v){
	return lCons(lValSym("begin"),v);
}
static inline lVal *lEvalCast(lClosure *c, lVal *v){
	lVal *t = lApply(c,v,lEval);
	return lCast(c,t,lTypecastList(t));
}
static inline lVal *lEvalCastSpecific(lClosure *c, lVal *v, const lType type){
	return lCast(c,lApply(c,v,lEval),type);
}
static inline lVal *lEvalCastNumeric(lClosure *c, lVal *v){
	lVal *t = lApply(c,v,lEval);
	lType type = lTypecastList(t);
	if(type == ltString){type = ltFloat;}
	return lCast(c,t,type);
}
static inline lVal *lLastCar(lVal *v){
	forEach(a,v){
		if(a->vList.cdr == NULL){
			return a->vList.car;
		}
	}
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
static inline uint lStringLength(lString *s){
	return s->bufEnd - s->buf;
}
static inline int lSymCmp(const lVal *a,const lVal *b){
	if((a == NULL) || (b == NULL)){return 2;}
	if((a->type != ltSymbol) || (b->type != ltSymbol)){return 2;}
	if(a->vSymbol.v[0] != b->vSymbol.v[0]){return -1;}
	if(a->vSymbol.v[1] != b->vSymbol.v[1]){return -1;}
	return 0;
}

#define lEvalCastIApply(FUNC, c , v) do { \
	if((c == NULL) || (v == NULL)){return lValInt(0);} \
	lVal *t = lEvalCastSpecific(c,v,ltInt); \
	if((t == NULL) || (t->type != ltPair)){return lValInt(0);} \
	lVal *d = lValDup(t->vList.car); \
	if(d == NULL){return lValInt(0);} \
	return FUNC(d,t); \
	} while (0)

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
