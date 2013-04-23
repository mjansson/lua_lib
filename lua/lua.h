/* lua.h  -  Lua library  -  MIT License  -  2013 Mattias Jansson / Rampant Pixels
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

/*! \file lua.h
    Lua scripting through customized LuaJIT implementation, for foundation based projects */

#include <foundation/platform.h>
#include <foundation/types.h>


#if defined( LUA_COMPILE ) && LUA_COMPILE
#  ifdef __cplusplus
#  define LUA_EXTERN extern "C"
#  else
#  define LUA_EXTERN extern
#  endif
#  define LUA_API
#else
#  ifdef __cplusplus
#  define LUA_EXTERN extern "C"
#  define LUA_API extern "C"
#  else
#  define LUA_EXTERN extern
#  define LUA_API extern
#  endif
#endif


struct lua_State;

typedef int (*lua_fn)( struct lua_State* );

//! Return codes
typedef enum _lua_result
{
	//! Call queued
	LUA_QUEUED                                       = 1,
	
	//! Continue script calls
	LUA_OK                                           = 0,

	//! Error in script evaluation
	LUA_ERROR                                        = -1,
	
	//! Stop further script evaluation
	LUA_STOP                                         = -2
} lua_result_t;

typedef enum _lua_data
{
	LUADATA_PTR    = 0,
	LUADATA_OBJ,
	LUADATA_INT,
	LUADATA_REAL,
	LUADATA_STR,
	LUADATA_BOOL,
	LUADATA_INTARR,
	LUADATA_REALARR
} lua_data_t;

typedef union _lua_value
{
	void*            ptr;
	object_t         obj;
	int              ival;
	real             val;
	const char*      str;
	bool             flag;
	lua_fn           fn;
} lua_value_t;

#define LUA_MAX_ARGS  8

typedef struct _lua_arg
{
	int32_t              num;
	uint8_t              type[LUA_MAX_ARGS];
	uint16_t             size[LUA_MAX_ARGS];
	lua_value_t          value[LUA_MAX_ARGS];
} lua_arg_t;

typedef struct _lua_environment lua_environment_t;

