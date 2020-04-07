/* compile.h  -  Lua library  -  Public Domain  -  2016 Mattias Jansson
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

/*! \file compile.h
    Lua library resource compilation */

#include <foundation/platform.h>
#include <resource/types.h>

#include <lua/types.h>

/* Compile lua resource
\param uuid Resource UUID
\param platform Resource platform
\param source Resource source representation
\param type Type string
\param type_length Length of type string
\return 0 if successful, <0 if error */
LUA_API int
lua_compile(const uuid_t uuid, uint64_t platform, resource_source_t* source, const uint256_t source_hash,
            const char* type, size_t type_length);
