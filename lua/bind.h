/* bind.h  -  Lua library  -  MIT License  -  2013 Mattias Jansson / Rampant Pixels
 *
 * This library provides a fork of the LuaJIT library with custom modifications for projects
 * based on our foundation library.
 *
 * The latest source code maintained by Rampant Pixels is always available at
 * https://github.com/rampantpixels/lua_lib
 *
 * For more information about LuaJIT, see
 * http://luajit.org/
 *
 * The MIT License (MIT)
 * Copyright (c) 2013 Rampant Pixels AB
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

/*! \file bind.h
    Utility macros and functions for interacting with Lua state */

#include <foundation/platform.h>

#include <lua/types.h>
#include <lua/luajit.h>

#define LUABIND_GET_CSTR(x)     (lua_tostring(state, (x)))
#define LUABIND_GET_INT(x)      ((int)lua_tointeger(state, (x)))
#define LUABIND_GET_UINT(x)     ((unsigned int)lua_tonumber(state, (x)))
#define LUABIND_GET_REAL(x)     ((real)lua_tonumber(state, (x)))
#define LUABIND_GET_BOOL(x)     (lua_toboolean(state, (x)))
#define LUABIND_GET_OBJECT(x)   (lua_toobject(state, (x)))
#define LUABIND_GET_DATA(t,x)   ((t*)lua_touserdata(state, (x)))

#define LUABIND_PUT_NIL()       (lua_pushnil(state))
#define LUABIND_PUT_CSTR(x)     (lua_pushstring(state, (x)))
#define LUABIND_PUT_INT(x)      (lua_pushinteger(state, (x)))
#define LUABIND_PUT_UINT(x)     (lua_pushinteger(state, (x)))
#define LUABIND_PUT_REAL(x)     (lua_pushnumber(state, (x)))
#define LUABIND_PUT_BOOL(x)     (lua_pushboolean(state, (x)))
#define LUABIND_PUT_OBJECT(x)   (lua_pushobject(state, (uint64_t)(x)))
#define LUABIND_PUT_DATA(x)     (lua_pushlightuserdata(state, (x)))

