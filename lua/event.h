/* event.h  -  Lua library  -  Public Domain  -  2016 Mattias Jansson
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

/*! \file event.h
    Lua library event handling */

#include <foundation/platform.h>

#include <lua/types.h>

/*! Handle resource events. No other event types should be
passed to this function.
\param event Resource event */
LUA_API void
lua_event_handle_resource(const event_t* event);
