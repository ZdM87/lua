/*
** $Id: lfunc.c,v 1.15 1999/11/22 13:12:07 roberto Exp roberto $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/


#include <stdlib.h>

#define LUA_REENTRANT

#include "lfunc.h"
#include "lmem.h"
#include "lstate.h"

#define gcsizeproto(L, p)	numblocks(L, 0, sizeof(TProtoFunc))
#define gcsizeclosure(L, c) numblocks(L, c->nelems, sizeof(Closure))



Closure *luaF_newclosure (lua_State *L, int nelems) {
  Closure *c = (Closure *)luaM_malloc(L, sizeof(Closure)+nelems*sizeof(TObject));
  c->next = L->rootcl;
  L->rootcl = c;
  c->marked = 0;
  c->nelems = nelems;
  L->nblocks += gcsizeclosure(L, c);
  return c;
}


TProtoFunc *luaF_newproto (lua_State *L) {
  TProtoFunc *f = luaM_new(L, TProtoFunc);
  f->code = NULL;
  f->lineDefined = 0;
  f->source = NULL;
  f->consts = NULL;
  f->nconsts = 0;
  f->locvars = NULL;
  f->next = L->rootproto;
  L->rootproto = f;
  f->marked = 0;
  L->nblocks += gcsizeproto(L, f);
  return f;
}


void luaF_freeproto (lua_State *L, TProtoFunc *f) {
  L->nblocks -= gcsizeproto(L, f);
  luaM_free(L, f->code);
  luaM_free(L, f->locvars);
  luaM_free(L, f->consts);
  luaM_free(L, f);
}


void luaF_freeclosure (lua_State *L, Closure *c) {
  L->nblocks -= gcsizeclosure(L, c);
  luaM_free(L, c);
}


/*
** Look for n-th local variable at line `line' in function `func'.
** Returns NULL if not found.
*/
const char *luaF_getlocalname (const TProtoFunc *func,
                               int local_number, int line) {
  int count = 0;
  const char *varname = NULL;
  LocVar *lv = func->locvars;
  if (lv == NULL)
    return NULL;
  for (; lv->line != -1 && lv->line < line; lv++) {
    if (lv->varname) {  /* register */
      if (++count == local_number)
        varname = lv->varname->str;
    }
    else  /* unregister */
      if (--count < local_number)
        varname = NULL;
  }
  return varname;
}

