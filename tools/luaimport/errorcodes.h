/* errorcodes.h  -  Lua library importer  -  Public Domain  -  2016 Mattias Jansson
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

//Error codes returned by luaimport tool
#define LUAIMPORT_RESULT_OK                           0
#define LUAIMPORT_RESULT_IMPORT_FAILED               -1
#define LUAIMPORT_RESULT_INVALID_ARGUMENTS           -2
