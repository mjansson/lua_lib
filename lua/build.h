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
