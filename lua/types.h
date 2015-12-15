/* types.h  -  Lua library  -  MIT License  -  2013 Mattias Jansson / Rampant Pixels
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

/*! \file types.h
    Data types used by Lua scripting library */

#include <foundation/platform.h>
#include <foundation/types.h>

#include <lua/build.h>

#if defined( LUA_COMPILE ) && LUA_COMPILE
#  ifdef __cplusplus
#  define LUA_EXTERN extern "C"
#  define LUA_API extern "C"
#  else
#  define LUA_EXTERN extern
#  define LUA_API extern
#  endif
#else
#  ifdef __cplusplus
#  define LUA_EXTERN extern "C"
#  define LUA_API extern "C"
#  else
#  define LUA_EXTERN extern
#  define LUA_API extern
#  endif
#endif

#define LUA_MAX_ARGS  8

//! Return codes
typedef enum {
	//! Call queued
	LUA_QUEUED                                       = 1,

	//! Continue script calls
	LUA_OK                                           = 0,

	//! Error in script evaluation
	LUA_ERROR                                        = -1,

	//! Stop further script evaluation
	LUA_STOP                                         = -2
} lua_result_t;

typedef enum {
	LUADATA_PTR    = 0,
	LUADATA_OBJ,
	LUADATA_INT,
	LUADATA_REAL,
	LUADATA_STR,
	LUADATA_BOOL,
	LUADATA_INTARR,
	LUADATA_REALARR
} lua_data_t;

typedef struct lua_State lua_State;
typedef int (*lua_fn)(struct lua_State*);

typedef union lua_value_t lua_value_t;

typedef struct lua_arg_t lua_arg_t;
typedef struct lua_readstream_t lua_readstream_t;
typedef struct lua_readbuffer_t lua_readbuffer_t;
typedef struct lua_readstring_t lua_readstring_t;
typedef struct lua_config_t lua_config_t;
typedef struct lua_t lua_t;

union lua_value_t {
	void*       ptr;
	object_t    obj;
	int         ival;
	real        val;
	const char* str;
	bool        flag;
	lua_fn      fn;
};

struct lua_config_t {
	unsigned int __unused;
};

struct lua_arg_t {
	int32_t     num;
	uint8_t     type[LUA_MAX_ARGS];
	uint16_t    size[LUA_MAX_ARGS];
	lua_value_t value[LUA_MAX_ARGS];
};

struct lua_readstream_t {
	stream_t* stream;
	char      chunk[512];
};

struct lua_readbuffer_t {
	const void* buffer;
	size_t      size;
	size_t      offset;
};

struct lua_readstring_t {
	const char* string;
	size_t      size;
};

struct lua_t
{
	//! Lua state
	lua_State*   state;

	//! Call depth
	int32_t      calldepth;

#if BUILD_ENABLE_LUA_THREAD_SAFE
	//! Call queue
	lua_op_t     queue[BUILD_LUA_CALL_QUEUE_SIZE];

	//! Queue head (protected by execute semaphore)
	uint32_t     queue_head;

	//! Queue tail
	atomic32_t   queue_tail;

	//! Execution right
	semaphore_t  execution_right;

	//! Currently executing thread
	atomic64_t   executing_thread;

	//! Execution count (protected by execute semaphore)
	unsigned int executing_count;
#endif

	//! Lookup hashmap
	hashmap_t    lookup_map;
};


