/* build.h  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

/*! \file build.h
\brief Build setup

Build setup. This header contains all the compile time build setup for the lua library. */

#include <foundation/platform.h>
#include <foundation/types.h>

/*! \def BUILD_ENABLE_LUA_THREAD_SAFE
Control if Lua states are thread safe, i.e calls to a lua state object can happen
concurrently across multiple threads. Introduces overhead by assigning execution right
on a lua state to one thread at a time. */
#define BUILD_ENABLE_LUA_THREAD_SAFE 0

/*! \def BUILD_LUA_CALL_QUEUE_SIZE
Number of calls that can be queued while synchronizing thread execution. Only used
if BUILD_ENABLE_LUA_THREAD_SAFE is set. */
#define BUILD_LUA_CALL_QUEUE_SIZE  256

#define BUILD_SIZE_LUA_LOOKUP_BUCKETS 31
#define BUILD_SIZE_LUA_NAME_MAXLENGTH 128

#define BUILD_REGISTRY_LOADED_MODULES "loaded-modules"
