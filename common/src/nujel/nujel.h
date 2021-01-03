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
		char c[8];
		uint64_t v;
	};
} lSymbol;

struct lVal {
	u32 flags;
	lType type;
	lVal *next;
	union {
		void         *vNil;
		bool          vBool;
		lVal         *vList;
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

void      lInit             ();
int       lMemUsage         ();
lClosure *lClosureNew       (lClosure *parent);
void      lClosureFree      (lClosure *c);
lVal     *lClosureAddNF     (lClosure *c, const char *sym, lVal *(*func)(lClosure *,lVal *));
void      lValFree          (lVal *v);
void      lClosureGC        ();
lVal     *lParseSExprCS     (const char *str);
void      lPrintVal         (lVal *v);
void      lPrintChain       (lVal *v);
char     *lSPrintVal        (lVal *v, char *buf, char *bufEnd);
char     *lSPrintChain      (lVal *v, char *buf, char *bufEnd);
lVal     *lResolveClosureSym(lClosure *c, const lSymbol s);
lVal     *lDefineClosureSym (lClosure *c, const lSymbol s);
lVal     *lResolveSym       (lClosure *c, const lSymbol s);
lVal     *lEval             (lClosure *c, lVal *v);
lType     lTypecast         (lVal *a, lVal *b);

lVal     *lValNil       ();
lVal     *lValList      (lVal *v);
lVal     *lValListS     (const char *s, lVal *v);
lVal     *lValBool      (bool v);
lVal     *lValInf       ();
lVal     *lValInt       (int v);
lVal     *lValFloat     (float v);
lVal     *lValVec       (const vec v);
lVal     *lValSym       (const char *s);
lVal     *lValString    (const char *s);
lVal     *lnfCat        (lClosure *c, lVal *v);
lVal     *lValDup       (const lVal *v);

extern char *ansiRS;
extern char *ansiFG[16];
