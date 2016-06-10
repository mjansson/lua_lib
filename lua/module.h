/* module.h  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

/*! \file module.h
    Registry for lua modules */

#include <foundation/platform.h>

#include <lua/types.h>

/*! Query if module is loaded in the given lua environment
\param lua Lua environment
\param uuid Module
\return true if loaded, false if not */
LUA_API bool
lua_module_is_loaded(lua_t* lua, const uuid_t uuid);

/*! Reload the given module into the given lua environment
\param lua Lua environment
\param uuid Module to reload
\return 0 if successful, <0 if error */
LUA_API int
lua_module_reload(lua_t* lua, const uuid_t uuid);

/*! Register a module with the module loader.
\param name Module name
\param length Length of module name
\param uuid Module UUID
\param loader Loader function (typically lua_module_loader)
\param preload Function to call before loading the module (typically to register symbol lookups) */
LUA_API void
lua_module_register(const char* name, size_t length, const uuid_t uuid, lua_fn loader, lua_preload_fn preload);

/*! Standard module loader. Takes the module entry as the first upvalue index as a
lua_modulemap_entry_t userdata pointer.
\param state Lua state
\return Number of return values on state stack */
LUA_API int
lua_module_loader(lua_State* state);
