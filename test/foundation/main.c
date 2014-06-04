/* main.c  -  Foundation bind test for lua library  -  MIT License  -  2013 Mattias Jansson / Rampant Pixels
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

#include <lua/lua.h>
#include <foundation/foundation.h>
#include <test/test.h>


application_t test_foundation_application( void )
{
	application_t app = {0};
	app.name = "Foundation tests";
	app.short_name = "test_lua_foundation";
	app.config_dir = "test_lua_foundation";
	app.flags = APPLICATION_UTILITY;
	return app;
}


memory_system_t test_foundation_memory_system( void )
{
	return memory_system_malloc();
}


int test_foundation_initialize( void )
{
	return 0;
}


void test_foundation_shutdown( void )
{
}


DECLARE_TEST( foundation, log )
{
	lua_t* env = lua_allocate();

	log_set_suppress( HASH_LUA, ERRORLEVEL_NONE );

	EXPECT_NE( env, 0 );

	const char* testcode =
	"local foundation = require(\"foundation\")\n"
	"local log = foundation.log\n"
	"local error = foundation.error\n"
	"log.set_suppress( \"lua\", error.LEVEL_NONE )\n"
	"log.debug( \"Testing log debug output\" )\n"
	"log.info( \"Testing log info output\" )\n"
	"log.warn( \"Testing log warning output\" )\n"
	"log.enable_prefix( false )\n"
	"log.error( \"Testing log error output without prefix\" )\n"
	"log.enable_stdout( false )\n"
	"log.debug( \"Invisible on stdout\" )\n"
	"log.enable_stdout( true )\n"
	"log.enable_prefix( true )\n"
	"log.set_suppress( \"lua\", error.LEVEL_INFO )\n";
	
	EXPECT_EQ( lua_eval_string( env, testcode ), LUA_OK );

	EXPECT_EQ( error(), ERROR_SCRIPT );
	
	lua_deallocate( env );
	
	return 0;
}


DECLARE_TEST( foundation, environment )
{
	lua_t* env = lua_allocate();

	log_set_suppress( HASH_LUA, ERRORLEVEL_NONE );
	log_info( HASH_LUA, "Running environment lua tests" );

	EXPECT_NE( env, 0 );

	const char* testcode =
	"local foundation = require( \"foundation\" )\n"
	"local ffi = require( \"ffi\" )\n"
	"local log = foundation.log\n"
	"local error = foundation.error\n"
	"local env = foundation.environment\n"
	"log.set_suppress( \"lua\", error.LEVEL_DEBUG )\n"
	"log.enable_prefix( false )\n"
	"log.info( \"Executable name: \" .. ffi.string( env.executable_name() ) )\n"
	"local cmdline = \"\"\n"
	"local cmdline_tab = env.command_line()\n"
	"local i = 1\n"
	"while cmdline_tab[i] ~= nil do\n"
	"  cmdline = cmdline .. \" \" .. cmdline_tab[i]\n"
	"  i = i + 1\n"
	"end\n"
	"log.info( \"Command line:\" .. cmdline )\n"
	"log.enable_prefix( true )\n"
	"log.set_suppress( \"lua\", error.LEVEL_INFO )\n";

	EXPECT_EQ( lua_eval_string( env, testcode ), LUA_OK );

	log_infof( HASH_LUA, "Done running environment lua tests" );
	log_set_suppress( HASH_LUA, ERRORLEVEL_INFO );
	
	lua_deallocate( env );

	return 0;
}


void test_foundation_declare( void )
{
	ADD_TEST( foundation, log );
	ADD_TEST( foundation, environment );
}


test_suite_t test_foundation_suite = {
	test_foundation_application,
	test_foundation_memory_system,
	test_foundation_declare,
	test_foundation_initialize,
	test_foundation_shutdown
};


#if FOUNDATION_PLATFORM_ANDROID

int test_foundation_run( void );
int test_foundation_run( void )
{
	test_suite = test_foundation_suite;
	return test_run_all();
}

#else

test_suite_t test_suite_define( void );
test_suite_t test_suite_define( void )
{
	return test_foundation_suite;
}

#endif
