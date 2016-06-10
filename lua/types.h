/* types.h  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

typedef enum {
	LUACMD_WAIT = 0,
	LUACMD_LOAD,
	LUACMD_EVAL,
	LUACMD_CALL,
	LUACMD_BIND,
	LUACMD_BIND_INT,
	LUACMD_BIND_VAL
} lua_command_t;

typedef struct lua_State lua_State;
typedef int (*lua_fn)(lua_State*);
typedef void (*lua_preload_fn)(void);

typedef union lua_value_t lua_value_t;

typedef struct lua_arg_t lua_arg_t;
typedef struct lua_op_t lua_op_t;
typedef struct lua_readstream_t lua_readstream_t;
typedef struct lua_readbuffer_t lua_readbuffer_t;
typedef struct lua_readstring_t lua_readstring_t;
typedef struct lua_modulemap_entry_t lua_modulemap_entry_t;
typedef struct lua_config_t lua_config_t;
typedef struct lua_t lua_t;

struct lua_config_t {
	unsigned int __unused;
};

union lua_value_t {
	void*       ptr;
	object_t    obj;
	int         ival;
	real        val;
	const char* str;
	bool        flag;
	lua_fn      fn;
};

struct lua_arg_t {
	int32_t     num;
	uint8_t     type[LUA_MAX_ARGS];
	uint16_t    size[LUA_MAX_ARGS];
	lua_value_t value[LUA_MAX_ARGS];
};

struct lua_op_t {
	lua_command_t         cmd;
	union {
		const char*       name;
		void*             ptr;
	} data;
	size_t                size;
	lua_arg_t             arg;
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

struct lua_t {
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
};

struct lua_modulemap_entry_t {
	hash_t name;
	uuid_t uuid;
	lua_fn loader;
	lua_preload_fn preload;
};

#define LUA_FOUNDATION_UUID uuid_make(0x4666006cd11e65efULL, 0x291433d0785ef08dULL)
