/* call.h  -  Lua library  -  Public Domain  -  2017 Mattias Jansson
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

/*! \file call.h
    Lua call */

#include <foundation/platform.h>

#include <lua/types.h>

//! Call method
LUA_API lua_result_t
lua_call_void(lua_t* env, const char* method, size_t length);

//! Call method
LUA_API lua_result_t
lua_call_ptr(lua_t* env, const char* method, size_t length, void* arg);

//! Call method
LUA_API lua_result_t
lua_call_object(lua_t* env, const char* method, size_t length, object_t arg);

//! Call method
LUA_API lua_result_t
lua_call_real(lua_t* env, const char* method, size_t length, real arg);

//! Call method
LUA_API lua_result_t
lua_call_int(lua_t* env, const char* method, size_t length, int arg);

//! Call method (arglength must fit in a uint16_t)
LUA_API lua_result_t
lua_call_string(lua_t* env, const char* method, size_t length, const char* arg, size_t arglength);

//! Call method
LUA_API lua_result_t
lua_call_bool(lua_t* env, const char* method, size_t length, bool arg);

//! Call method
LUA_API lua_result_t
lua_call_custom(lua_t* env, const char* method, size_t length, lua_arg_t* arg);
