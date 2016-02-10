/* read.h  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

/*! \file read.h
    Callbacks for reading lua code */

#include <foundation/platform.h>

#include <lua/types.h>

LUA_API const char*
lua_read_stream(lua_State* state, void* user_data, size_t* size);

LUA_API const char*
lua_read_chunked_buffer(lua_State* state, void* user_data, size_t* size);

LUA_API const char*
lua_read_buffer(lua_State* state, void* user_data, size_t* size);

LUA_API const char*
lua_read_string(lua_State* state, void* user_data, size_t* size);

