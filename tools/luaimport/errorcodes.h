/* errorcodes.h  -  Lua library importer  -  Public Domain  -  2016 Mattias Jansson / Rampant Pixels
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

//Error codes returned by luaimport tool
#define LUAIMPORT_RESULT_OK                           0
#define LUAIMPORT_RESULT_UNSUPPORTED_INPUT           -1
#define LUAIMPORT_RESULT_INVALID_ARGUMENT            -2
#define LUAIMPORT_RESULT_UNKNOWN_COMMAND             -3
#define LUAIMPORT_RESULT_UNABLE_TO_OPEN_OUTPUT_FILE  -4
#define LUAIMPORT_RESULT_INVALID_INPUT               -5
#define LUAIMPORT_RESULT_UNABLE_TO_OPEN_MAP_FILE     -6
#define LUAIMPORT_RESULT_UNABLE_TO_WRITE_BLOB        -7
#define LUAIMPORT_RESULT_UNABLE_TO_WRITE_SOURCE      -8
#define LUAIMPORT_RESULT_UUID_MISMATCH               -9

#define LUAIMPORT_RESULT_FAILED_EVAL                 -100
