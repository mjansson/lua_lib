/* import.h  -  Lua library  -  Public Domain  -  2016 Mattias Jansson / Rampant Pixels
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

/*! \file import.h
    Lua library resource import */

#include <foundation/platform.h>
#include <resource/types.h>

#include <lua/types.h>

/* Import lua source to a resource
\param stream Source stream
\param uuid Resource UUID
\return 0 if successful, <0 if error */
int
lua_import(stream_t* stream, const uuid_t uuid);
