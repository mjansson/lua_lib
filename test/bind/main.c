/* main.c  -  Foundation bind test for lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <foundation/foundation.h>
#include <resource/resource.h>
#include <lua/lua.h>
#include <test/test.h>

application_t
test_bind_application(void) {
	application_t app;
	memset(&app, 0, sizeof(app));
	app.name = string_const(STRING_CONST("Lua bind tests"));
	app.short_name = string_const(STRING_CONST("test_lua_bind"));
	app.company = string_const(STRING_CONST("Rampant Pixels"));
	app.flags = APPLICATION_UTILITY;
	app.exception_handler = test_exception_handler;
	return app;
}

memory_system_t
test_bind_memory_system(void) {
	return memory_system_malloc();
}

foundation_config_t
test_bind_config(void) {
	foundation_config_t config;
	memset(&config, 0, sizeof(config));
	return config;
}

int
test_bind_initialize(void) {
	lua_config_t lua_config;
	resource_config_t resource_config;

	memset(&lua_config, 0, sizeof(lua_config));
	memset(&resource_config, 0, sizeof(resource_config));

	resource_config.enable_local_source = true;
	resource_config.enable_local_cache = true;
	resource_config.enable_remote_cache = true;

	if (resource_module_initialize(resource_config) < 0)
		return -1;

	return lua_module_initialize(lua_config);
}

void
test_bind_finalize(void) {
	lua_module_finalize();
	resource_module_finalize();
}

DECLARE_TEST(bind, bind) {
	lua_t* env = lua_allocate();

	log_set_suppress(HASH_LUA, ERRORLEVEL_NONE);

	EXPECT_NE(env, 0);




	lua_deallocate(env);

	return 0;
}

void
test_bind_declare(void) {
	ADD_TEST(bind, bind);
}

test_suite_t test_bind_suite = {
	test_bind_application,
	test_bind_memory_system,
	test_bind_config,
	test_bind_declare,
	test_bind_initialize,
	test_bind_finalize
};

#if FOUNDATION_PLATFORM_ANDROID || FOUNDATION_PLATFORM_IOS

int
test_bind_run(void);

int
test_bind_run(void) {
	test_suite = test_bind_suite;
	return test_run_all();
}

#else

test_suite_t
test_suite_define(void);

test_suite_t
test_suite_define(void) {
	return test_bind_suite;
}

#endif
