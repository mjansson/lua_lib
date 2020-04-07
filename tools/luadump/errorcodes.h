/* errorcodes.h  -  Lua dump tool for lua library  -  Public Domain  -  2013 Mattias Jansson
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

//Error codes returned by luadump tool
#define LUADUMP_RESULT_OK                              0
#define LUADUMP_RESULT_UNABLE_TO_OPEN_INPUT_FILE      -1
#define LUADUMP_RESULT_UNABLE_TO_OPEN_OUTPUT_FILE     -2
#define LUADUMP_RESULT_UNABLE_TO_LOAD_JITLIB_VMDEF    -3
#define LUADUMP_RESULT_UNABLE_TO_LOAD_JITLIB_BC       -4
#define LUADUMP_RESULT_FAILED_EVAL                    -5
#define LUADUMP_RESULT_UNABLE_TO_LOAD_FOUNDATION      -6
