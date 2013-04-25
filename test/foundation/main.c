/* main.c  -  Lua foundation tests  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
 *
 * This library provides a cross-platform vector math library in C11 providing basic support data types and
 * functions to write applications and games in a platform-independent fashion. The latest source code is
 * always available at
 *
 * https://github.com/rampantpixels/vector_lib
 *
 * This library is built on top of the foundation library available at
 *
 * https://github.com/rampantpixels/foundation_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without any restrictions.
 *
 */

#include <lua/lua.h>
#include <foundation/foundation.h>
#include <test/test.h>


application_t test_application( void )
{
	application_t app = {0};
	app.name = "Foundation tests";
	app.short_name = "test_lua_foundation";
	app.config_dir = "test_lua_foundation";
	app.flags = APPLICATION_UTILITY;
	return app;
}


DECLARE_TEST( foundation, log )
{
	lua_environment_t* env = lua_environment_allocate();

	const char* testcode =
	"log.debug( \"Testing log debug output\" )\n"
	"log.info( \"Testing log info output\" )\n"
	"log.warn( \"Testing log warning output\" )\n"
	"log.error( \"Testing log error output\" )\n";
	
	lua_eval( env, testcode );

	lua_environment_deallocate( env );
	
	return 0;
}


void test_declare( void )
{
	ADD_TEST( foundation, log );
}

