/* common.c */
/* (c) Reuben Thomas 2000-2006 */
/* (c) Shmuel Zeigerman 2004-2006 */

#include <stdlib.h>
#include <mem.h>

#include "lua.h"
#include "lauxlib.h"

#include "common.h"

void *Lmalloc(lua_State *L, size_t size)
{
  void *p = malloc(size);
  if(p == NULL)
    luaL_error(L, "malloc failed");
  return p;
}

int get_startoffset(lua_State *L, int stackpos, size_t len)
{
  int startoffset = luaL_optint(L, stackpos, 1);
  if(startoffset > 0)
    startoffset--;
  else if(startoffset < 0) {
    startoffset += len;
    if(startoffset < 0)
      startoffset = 0;
  }
  return startoffset;
}

int udata_tostring(lua_State *L, const char* type_handle, const char* type_name)
{
  char buf[256];
  void *udata = luaL_checkudata(L, 1, type_handle);
  if(udata) {
    sprintf(buf, "%s (%p)", type_name, udata);
    lua_pushstring(L, buf);
  }
  else {
    sprintf(buf, "must be userdata of type '%s'", type_name);
    luaL_argerror(L, 1, buf);
  }
  return 1;
}

/* This function fills a table with string-number pairs.
   The table can be passed as the 1-st lua-function parameter,
   otherwise it is created. The return value is the filled table.
*/
int get_flags (lua_State *L, const flag_pair *arr)
{
  const flag_pair *p;
  int nparams = lua_gettop(L);

  if(nparams == 0)
    lua_newtable(L);
  else {
    if(!lua_istable(L, 1))
      luaL_argerror(L, 1, "not a table");
    if(nparams > 1)
      lua_pushvalue(L, 1);
  }

  for(p=arr; p->key != NULL; p++) {
    lua_pushstring(L, p->key);
    lua_pushinteger(L, p->val);
    lua_rawset(L, -3);
  }
  return 1;
}

void createmeta(lua_State *L, const char *name)
{
  luaL_newmetatable(L, name);   /* create new metatable */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);         /* push metatable */
  lua_rawset(L, -3);            /* metatable.__index = metatable, for OO-style use */
}

void CheckStack (lua_State *L, int extraslots)
{
  if (lua_checkstack (L, extraslots) == 0)
    luaL_error (L, "cannot add %d stack slots", extraslots);
}

int CheckFunction (lua_State *L, int pos) {
  luaL_checktype (L, pos, LUA_TFUNCTION);
  return pos;
}

int OptFunction (lua_State *L, int pos) {
  if (lua_isnoneornil (L, pos))
    return 0;
  luaL_checktype (L, pos, LUA_TFUNCTION);
  return pos;
}

/* function plainfind (s, p, [st], [ci]) */
int plainfind_func (lua_State *L) {
  typedef int (*f_comp)(const void*, const void*, size_t);
  size_t textlen, patlen;

  const char *text = luaL_checklstring (L, 1, &textlen);
  const char *pattern = luaL_checklstring (L, 2, &patlen);
  const char *from = text + get_startoffset (L, 3, textlen);
  f_comp func = lua_toboolean (L, 4) ? memicmp : memcmp;
  const char *end = text + textlen;

  for (; from + patlen <= end; ++from) {
    if (!(*func) (from, pattern, patlen)) {
      lua_pushinteger (L, from - text + 1);
      lua_pushinteger (L, from - text + patlen);
      return 2;
    }
  }
  lua_pushnil (L);
  return 1;
}

