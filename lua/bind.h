/* bind.h  -  Lua library  -  Public Domain  -  2013 Mattias Jansson
 *
 * This library provides a cross-platform lua library in C11 for games and applications
 * based on out foundation library. The latest source code is always available at
 *
 * https://github.com/mjansson/lua_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without
 * any restrictions.
 *
 * The LuaJIT library is released under the MIT license. For more information about LuaJIT, see
 * http://luajit.org/
 */

#pragma once

/*! \file bind.h
    Utility macros and functions for interacting with Lua state */

#include <foundation/platform.h>

#include <lua/types.h>
#include <lua/luajit.h>

#define LUABIND_GET_CSTR(x) (lua_tostring(state, (x)))
#define LUABIND_GET_STRING(s, x) ((s).str = lua_tolstring(state, (x), &(s).length))
#define LUABIND_GET_INT(x) ((int)lua_tointeger(state, (x)))
#define LUABIND_GET_UINT(x) ((unsigned int)lua_tonumber(state, (x)))
#define LUABIND_GET_REAL(x) ((real)lua_tonumber(state, (x)))
#define LUABIND_GET_BOOL(x) (lua_toboolean(state, (x)))
#define LUABIND_GET_OBJECT(x) (lua_toobject(state, (x)))
#define LUABIND_GET_DATA(t, x) ((t*)lua_touserdata(state, (x)))

#define LUABIND_PUT_NIL() (lua_pushnil(state))
#define LUABIND_PUT_CSTR(x) (lua_pushstring(state, (x)))
#define LUABIND_PUT_STRING(x) (lua_pushlstring(state, (x).str, (x).length))
#define LUABIND_PUT_INT(x) (lua_pushinteger(state, (x)))
#define LUABIND_PUT_UINT(x) (lua_pushinteger(state, (x)))
#define LUABIND_PUT_REAL(x) (lua_pushnumber(state, (x)))
#define LUABIND_PUT_BOOL(x) (lua_pushboolean(state, (x)))
#define LUABIND_PUT_OBJECT(x) (lua_pushobject(state, (uint64_t)(x)))
#define LUABIND_PUT_DATA(x) (lua_pushlightuserdata(state, (x)))

//! Bind custom function
LUA_API lua_result_t
lua_bind_function(lua_t* env, const char* method, size_t length, lua_fn fn);

//! Bind custom integer
LUA_API lua_result_t
lua_bind_int(lua_t* env, const char* property, size_t length, int value);

//! Bind custom value
LUA_API lua_result_t
lua_bind_real(lua_t* env, const char* property, size_t length, real value);
