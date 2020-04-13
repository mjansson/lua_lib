/* lua.h  -  Lua library  -  Public Domain  -  2013 Mattias Jansson
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

/*! \file lua.h
    Lua scripting environment */

#include <foundation/platform.h>

#include <lua/types.h>
#include <lua/hashstrings.h>
#include <lua/bind.h>
#include <lua/module.h>
#include <lua/symbol.h>
#include <lua/event.h>
#include <lua/read.h>
#include <lua/import.h>
#include <lua/compile.h>
#include <lua/eval.h>
#include <lua/call.h>

#include <lua/foundation.h>
#include <lua/network.h>
#include <lua/resource.h>
#include <lua/window.h>

LUA_API int
lua_module_initialize(const lua_config_t config);

LUA_API void
lua_module_finalize(void);

LUA_API bool
lua_module_is_initialized(void);

LUA_API version_t
lua_module_version(void);

LUA_API lua_config_t
lua_module_config(void);

/*! Parse config declarations from JSON buffer
\param path Path
\param path_size Length of path
\param buffer Data buffer
\param size Size of data buffer
\param tokens JSON tokens
\param tokens_count Number of JSON tokens */
LUA_API void
lua_module_parse_config(const char* path, size_t path_size, const char* buffer, size_t size, const json_token_t* tokens,
                        size_t tokens_count);

//! Allocate environment
LUA_API lua_t*
lua_allocate(void);

//! Shutdown and free resources
LUA_API void
lua_deallocate(lua_t* env);

//! Get environment associated with lua state
LUA_API lua_t*
lua_from_state(lua_State* state);

//! Get lua state associated with environment
LUA_API lua_State*
lua_state(lua_t* env);

//! Access value
LUA_API string_const_t
lua_get_string(lua_t* env, const char* property, size_t length);

//! Access value
LUA_API int
lua_get_int(lua_t* env, const char* property, size_t length);

//! Execute queued operations and optionally run gc for specified amount of time
LUA_API void
lua_execute(lua_t* env, int gc_time, bool force);

//! Garbage collection, run gc for specified amount of time
LUA_API void
lua_timed_gc(lua_t* env, int milliseconds);

LUA_API int
lua_is_fr2(void);

LUA_API int
lua_arch_is_fr2(int arch);

#if BUILD_ENABLE_LUA_THREAD_SAFE

bool
lua_has_execution_right(lua_t* env);

bool
lua_acquire_execution_right(lua_t* env, bool force);

void
lua_release_execution_right(lua_t* env);

void
lua_push_op(lua_t* env, lua_op_t* op);

void
lua_execute_pending(lua_t* env);

#else

#define lua_has_execution_right(env) ((void)sizeof(env)), true
#define lua_acquire_execution_right(env, force) ((void)sizeof(env)), ((void)sizeof(force))
#define lua_release_execution_right(env) ((void)sizeof(env))
#define lua_push_op(env, op) ((void)sizeof(env)), ((void)sizeof(op))
#define lua_execute_pending(env) ((void)sizeof(env))

#endif
