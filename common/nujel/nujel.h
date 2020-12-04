#pragma once

typedef enum lType {
	ltNoAlloc = 0, ltNil, ltBool, ltList, ltLambda, ltClosure, ltInt, ltString, ltSymbol, ltNativeFunc, ltInf
} lType;

typedef struct lVal     lVal;
typedef struct lClosure lClosure;
typedef struct lString  lString;

typedef struct {
	union {
		char c[8];
		unsigned long long int v;
	};
} lSymbol;

struct lVal {
	unsigned int flags;
	lType type;
	lVal *next;
	union {
		void         *vNil;
		int           vBool;
		lVal         *vList;
		lVal         *vLambda;
		int           vInt;
		unsigned int  vUint;
		unsigned int  vChar;
		lString      *vString;
		lClosure     *vClosure;
		lSymbol       vSymbol;
		lVal       *(*vNativeFunc)(lClosure *, lVal *);
	};
};
#define lfMarked (1)

struct lClosure {
	lClosure *parent;
	lVal *data;
};

struct lString {
	char *buf,*data;
	union {
		char *bufEnd;
		lString *next;
	};
	int len;
};


void      lInit();
int       lMemUsage     ();
lClosure *lClosureNew   (lClosure *parent);
void      lClosureFree  (lClosure *c);
lVal     *lClosureAddNF (lClosure *c, const char *sym, lVal *(*func)(lClosure *,lVal *));
void      lValFree      (lVal *v);
void      lClosureGC    (lClosure *c);
lVal     *lParseSExprCS (char *str);
void      lPrintChain   (lVal *v);
char     *lSPrintVal    (lVal *v, char *buf, char *bufEnd);
char     *lSPrintChain  (lVal *v, char *buf, char *bufEnd);
lVal     *lResolveSym   (lClosure *c, const lSymbol s);
lVal     *lEval         (lClosure *c, lVal *v);
