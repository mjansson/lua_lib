/* luajit.h  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
 * 
 * This library provides a cross-platform lua library in C11 for games and applications
 * based on out foundation library. The latest source code is always available at
 *
 * https://github.com/rampantpixels/lua_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without
 * any restrictions.
 *
 * The LuaJIT library is released under the MIT license. For more information about LuaJIT, see
 * http://luajit.org/
 */

#pragma once

/*! \file luajit.h
    Stripped down internal Lua definitions for binding access */

#include <foundation/platform.h>
#include <foundation/types.h>

#include <lua/types.h>

#ifndef LUA_USE_INTERNAL_HEADER

#define LUA_NUMBER         double
#define LUA_INTEGER        uintptr_t

typedef LUA_NUMBER         lua_Number;
typedef LUA_INTEGER        lua_Integer;

typedef int                (*lua_CFunction) (lua_State *L);
typedef const char *       (*lua_Reader) (lua_State *L, void *ud, size_t *sz);
typedef int                (*lua_Writer) (lua_State *L, const void* p, size_t sz, void* ud);

LUA_EXTERN int             (lua_gettop) (lua_State *L);
LUA_EXTERN void            (lua_settop) (lua_State *L, int idx);
LUA_EXTERN void            (lua_pushvalue) (lua_State *L, int idx);
LUA_EXTERN void            (lua_remove) (lua_State *L, int idx);
LUA_EXTERN void            (lua_insert) (lua_State *L, int idx);
LUA_EXTERN void            (lua_replace) (lua_State *L, int idx);
LUA_EXTERN int             (lua_checkstack) (lua_State *L, int sz);

LUA_EXTERN void            (lua_xmove) (lua_State *from, lua_State *to, int n);

LUA_EXTERN int             (lua_isnumber) (lua_State *L, int idx);
LUA_EXTERN int             (lua_isstring) (lua_State *L, int idx);
LUA_EXTERN int             (lua_iscfunction) (lua_State *L, int idx);
LUA_EXTERN int             (lua_isuserdata) (lua_State *L, int idx);
LUA_EXTERN int             (lua_type) (lua_State *L, int idx);
LUA_EXTERN const char*     (lua_typename) (lua_State *L, int tp);

LUA_EXTERN int             (lua_equal) (lua_State *L, int idx1, int idx2);
LUA_EXTERN int             (lua_rawequal) (lua_State *L, int idx1, int idx2);
LUA_EXTERN int             (lua_lessthan) (lua_State *L, int idx1, int idx2);

LUA_EXTERN lua_Number      (lua_tonumber) (lua_State *L, int idx);
LUA_EXTERN lua_Integer     (lua_tointeger) (lua_State *L, int idx);
LUA_EXTERN uint64_t        (lua_toobject) (lua_State *L, int idx);
LUA_EXTERN int             (lua_toboolean) (lua_State *L, int idx);
LUA_EXTERN const char*     (lua_tolstring) (lua_State *L, int idx, size_t *len);
LUA_EXTERN size_t          (lua_objlen) (lua_State *L, int idx);
LUA_EXTERN lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);
LUA_EXTERN void*           (lua_touserdata) (lua_State *L, int idx);
LUA_EXTERN lua_State*      (lua_tothread) (lua_State *L, int idx);
LUA_EXTERN const void*     (lua_topointer) (lua_State *L, int idx);

LUA_EXTERN void            (lua_pushnil) (lua_State *L);
LUA_EXTERN void            (lua_pushnumber) (lua_State *L, lua_Number n);
LUA_EXTERN void            (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_EXTERN void            (lua_pushobject) (lua_State *L, uint64_t n);
LUA_EXTERN void            (lua_pushlstring) (lua_State *L, const char *s, size_t l);
LUA_EXTERN void            (lua_pushstring) (lua_State *L, const char *s);
LUA_EXTERN const char*     (lua_pushvfstring) (lua_State *L, const char *fmt,
                                                      va_list argp);
LUA_EXTERN const char*     (lua_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_EXTERN void            (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_EXTERN void            (lua_pushboolean) (lua_State *L, int b);
LUA_EXTERN void            (lua_pushlightuserdata) (lua_State *L, void *p);
LUA_EXTERN int             (lua_pushthread) (lua_State *L);

LUA_EXTERN void            (lua_gettable) (lua_State *L, int idx);
LUA_EXTERN void            (lua_getfield) (lua_State *L, int idx, const char *k);
LUA_EXTERN void            (lua_getlfield) (lua_State *L, int idx, const char *k, size_t n);
LUA_EXTERN void            (lua_rawget) (lua_State *L, int idx);
LUA_EXTERN void            (lua_rawgeti) (lua_State *L, int idx, int n);
LUA_EXTERN void            (lua_createtable) (lua_State *L, int narr, int nrec);
LUA_EXTERN void*           (lua_newuserdata) (lua_State *L, size_t sz);
LUA_EXTERN int             (lua_getmetatable) (lua_State *L, int objindex);
LUA_EXTERN void            (lua_getfenv) (lua_State *L, int idx);

LUA_EXTERN void            (lua_settable) (lua_State *L, int idx);
LUA_EXTERN void            (lua_setfield) (lua_State *L, int idx, const char *k);
LUA_EXTERN void            (lua_setlfield) (lua_State *L, int idx, const char *k, size_t n);
LUA_EXTERN void            (lua_rawset) (lua_State *L, int idx);
LUA_EXTERN void            (lua_rawseti) (lua_State *L, int idx, int n);
LUA_EXTERN int             (lua_setmetatable) (lua_State *L, int objindex);
LUA_EXTERN int             (lua_setfenv) (lua_State *L, int idx);

LUA_EXTERN int             (lua_is_fr2) (void);

#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))

#define LUA_YIELD	        1
#define LUA_ERRRUN	        2
#define LUA_ERRSYNTAX	    3
#define LUA_ERRMEM	        4
#define LUA_ERRERR	        5

#define LUA_TNONE		    (-1)

#define LUA_TNIL		    0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		    3
#define LUA_TSTRING		    4
#define LUA_TTABLE		    5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		    8


#define lua_pop(L,n)              lua_settop(L, -(n)-1)

#define lua_newtable(L)	          lua_createtable(L, 0, 0)

#define lua_register(L,n,f)       (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#define lua_pushcfunction(L,f)    lua_pushcclosure(L, (f), 0)

#define lua_strlen(L,i)           lua_objlen(L, (i))

#define lua_isfunction(L,n)       (lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)          (lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)  (lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)            (lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)        (lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)         (lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)           (lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)     (lua_type(L, (n)) <= 0)

#define lua_pushliteral(L, s)     lua_pushlstring(L, "" s, (sizeof(s)/sizeof(char))-1)

#define lua_setglobal(L,s)        lua_setfield(L, LUA_GLOBALSINDEX, (s))
#define lua_getglobal(L,s)        lua_getfield(L, LUA_GLOBALSINDEX, (s))
#define lua_setlglobal(L,s,n)     lua_setlfield(L, LUA_GLOBALSINDEX, (s), (n))
#define lua_getlglobal(L,s,n)     lua_getlfield(L, LUA_GLOBALSINDEX, (s), (n))

#define lua_tostring(L,i)         lua_tolstring(L, (i), 0)

/******************************************************************************
* Copyright (C) 1994-2008 Lua.org, PUC-Rio.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#endif
