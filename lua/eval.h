/* eval.h  -  Lua library  -  Public Domain  -  2013 Mattias Jansson
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

/*! \file eval.h
    Lua evaluation */

#include <foundation/platform.h>

#include <lua/types.h>

//! Load code from string
LUA_API lua_result_t
lua_eval_string(lua_t* env, const char* code, size_t length);

//! Load code from stream
LUA_API lua_result_t
lua_eval_stream(lua_t* env, stream_t* stream);

//! Load code from resource
LUA_API lua_result_t
lua_eval_resource(lua_t* env, const uuid_t uuid);
