/* environment.h  -  Lua library  -  MIT License  -  2013 Mattias Jansson / Rampant Pixels
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

/*! \file environment.h
    Lua scripting environment */

#include <foundation/platform.h>

#include "lua.h"


//! Allocate environment
LUA_API lua_environment_t*               lua_environment_allocate( void );

//! Shutdown and free resources
LUA_API void                             lua_environment_deallocate( lua_environment_t* env );

//! Bind custom method
LUA_API lua_result_t                     lua_bind( lua_environment_t* env, const char* method, lua_fn fn );

//! Bind custom integer
LUA_API lua_result_t                     lua_bind_int( lua_environment_t* env, const char* property, int value );

//! Bind custom value
LUA_API lua_result_t                     lua_bind_value( lua_environment_t* env, const char* property, real value );

//! Evaluate code
LUA_API lua_result_t                     lua_eval( lua_environment_t* env, const char* code );

//! Call method
LUA_API lua_result_t                     lua_call_void( lua_environment_t* env, const char* method );

//! Call method
LUA_API lua_result_t                     lua_call_ptr( lua_environment_t* env, const char* method, void* arg );

//! Call method
LUA_API lua_result_t                     lua_call_object( lua_environment_t* env, const char* method, object_t arg );

//! Call method
LUA_API lua_result_t                     lua_call_real( lua_environment_t* env, const char* method, real arg );

//! Call method
LUA_API lua_result_t                     lua_call_int( lua_environment_t* env, const char* method, int arg );

//! Call method
LUA_API lua_result_t                     lua_call_string( lua_environment_t* env, const char* method, const char* arg );

//! Call method
LUA_API lua_result_t                     lua_call_bool( lua_environment_t* env, const char* method, bool arg );

//! Call method
LUA_API lua_result_t                     lua_call_custom( lua_environment_t* env, const char* method, lua_arg_t* arg );

//! Access value
LUA_API const char*                      lua_get_string( lua_environment_t* env, const char* property );

//! Access value
LUA_API int                              lua_get_int( lua_environment_t* env, const char* property );

//! Execute queued operations and optionally run gc for specified amount of time
LUA_API void                             lua_execute( lua_environment_t* env, int gc_time, bool force );

//! Garbage collection, run gc for specified amount of time
LUA_API void                             lua_timed_gc( lua_environment_t* env, int milliseconds );

