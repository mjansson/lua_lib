/* lua.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson
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

#define LUA_USE_INTERNAL_HEADER

#include <lua/lua.h>

#include <foundation/foundation.h>
#include <resource/import.h>
#include <resource/compile.h>
#include <resource/stream.h>

#include <setjmp.h>

#undef LUA_API
#define LUA_HAS_LUA_STATE_TYPE

#include "luajit/src/lua.h"
#include "luajit/src/lauxlib.h"
#include "luajit/src/lualib.h"

#if !FOUNDATION_PLATFORM_WINDOWS
#include <sys/mman.h>
#endif

#undef LUA_API

static lua_config_t _lua_config;
static lua_t** _lua_instances;

lua_t**
lua_instances(void);

extern lua_result_t
lua_do_bind(lua_t* env, const char* property, size_t length, lua_command_t cmd, lua_value_t val);

extern lua_result_t
lua_do_call_custom(lua_t* env, const char* method, size_t length, lua_arg_t* arg);

extern lua_result_t
lua_do_eval_string(lua_t* env, const char* code, size_t length);

extern lua_result_t
lua_do_eval_stream(lua_t* env, stream_t* stream, uint64_t size_limit);

extern lua_result_t
lua_do_eval_uuid(lua_t* env, const uuid_t uuid);

static lua_result_t
lua_do_get(lua_t* env, const char* property, size_t length);

LUA_EXTERN void
lua_module_registry_finalize(lua_State* state);

LUA_EXTERN void
lua_module_registry_initialize(lua_State* state);

#if BUILD_ENABLE_LUA_THREAD_SAFE

bool
lua_has_execution_right(lua_t* env) {
	return (atomic_load64(&env->executing_thread) == thread_id());
}

bool
lua_acquire_execution_right(lua_t* env, bool force) {
	uint64_t self = thread_id();
	if (atomic_load64(&env->executing_thread) == self) {
		++env->executing_count;
		return true;
	}
	if (force) {
		semaphore_wait(&env->execution_right);
		atomic_store64(&env->executing_thread, self);
		FOUNDATION_ASSERT(env->executing_count == 0);
		++env->executing_count;
		return true;
	}
	if (semaphore_try_wait(&env->execution_right, 0)) {
		atomic_store64(&env->executing_thread, self);
		FOUNDATION_ASSERT(env->executing_count == 0);
		++env->executing_count;
		return true;
	}
	return false;
}

void
lua_release_execution_right(lua_t* env) {
	FOUNDATION_ASSERT(atomic_load64(&env->executing_thread) == thread_id());
	FOUNDATION_ASSERT(env->executing_count > 0);
	if (!--env->executing_count) {
		atomic_store64(&env->executing_thread, 0);
		semaphore_post(&env->execution_right);
	}
}

void
lua_push_op(lua_t* env, lua_op_t* op) {
	unsigned int ofs, old;
	do {
		old = atomic_load32(&env->queue_tail);
		ofs = old + 1;
		if (ofs >= BUILD_LUA_CALL_QUEUE_SIZE)
			ofs = 0;
	} while (!atomic_cas32(&env->queue_tail, ofs, old));

	// Got slot, copy except command
	memcpy(&env->queue[ofs].data, &op->data, sizeof(op->data) + sizeof(lua_arg_t));
	// Now set command, completing insert
	env->queue[ofs].cmd = op->cmd;
}

void
lua_execute_pending(lua_t* env) {
	profile_begin_block(STRING_CONST("lua exec"));

	unsigned int head = env->queue_head;
	while (env->queue[head].cmd != LUACMD_WAIT) {
		// Execute
		switch (env->queue[head].cmd) {
			case LUACMD_LOAD:
				lua_do_eval_stream(env, env->queue[head].data.ptr, 0);
				break;

			case LUACMD_LOAD_RESOURCE:
				lua_do_eval_uuid(env, env->queue[head].data.ptr);
				break;

			case LUACMD_EVAL:
				lua_do_eval_string(env, env->queue[head].data.name, env->queue[head].size);
				break;

			case LUACMD_CALL:
				lua_do_call_custom(env, env->queue[head].data.name, env->queue[head].size, &env->queue[head].arg);
				break;

			case LUACMD_BIND:
			case LUACMD_BIND_INT:
			case LUACMD_BIND_VAL:
				lua_do_bind(env, env->queue[head].data.name, env->queue[head].size, env->queue[head].cmd,
				            env->queue[head].arg.value[0]);
				break;

			default:
				break;
		}

		// Mark as executed
		env->queue[head].cmd = LUACMD_WAIT;

		if (++head == BUILD_LUA_CALL_QUEUE_SIZE)
			head = 0;
	}
	env->queue_head = head;

	profile_end_block();
}

#endif

static FOUNDATION_FORCEINLINE void
lua_run_gc(lua_t* env, int milliseconds) {
	lua_gc(env->state, LUA_GCSTEP, milliseconds);
}

#if FOUNDATION_PLATFORM_WINDOWS
#include <foundation/windows.h>
typedef long (*NtAllocateVirtualMemoryFn)(HANDLE, PVOID*, ULONG, SIZE_T*, ULONG, ULONG);
typedef long (*NtFreeVirtualMemoryFn)(HANDLE, PVOID*, SIZE_T*, ULONG);
static NtAllocateVirtualMemoryFn NtAllocateVirtualMemory;
static NtFreeVirtualMemoryFn NtFreeVirtualMemory;
#endif

static FOUNDATION_NOINLINE void*
lua_allocator(void* env, void* block, size_t osize, size_t nsize) {
	if (!nsize && osize) {
#if FOUNDATION_SIZE_POINTER == 8
		if (!lua_is_fr2()) {
#if FOUNDATION_PLATFORM_WINDOWS
			if (!NtFreeVirtualMemory)
				NtFreeVirtualMemory =
				    (NtFreeVirtualMemoryFn)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtFreeVirtualMemory");
			if (NtFreeVirtualMemory) {
				SIZE_T old_size = 0;
				NtFreeVirtualMemory(INVALID_HANDLE_VALUE, &block, &old_size, MEM_RELEASE);
			}
#else
			munmap(block, osize);
#endif
		} else
#endif
		{
			memory_deallocate(block);
		}
	} else if (nsize) {
#if FOUNDATION_SIZE_POINTER == 8
		if (!lua_is_fr2()) {
			void* raw_memory = 0;
			// Non-performance path, used only for compatibility tools
#if FOUNDATION_PLATFORM_WINDOWS
			size_t allocate_size = nsize;
			if (!NtAllocateVirtualMemory)
				NtAllocateVirtualMemory =
				    (NtAllocateVirtualMemoryFn)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtAllocateVirtualMemory");
			long vmres = NtAllocateVirtualMemory ?
			                 NtAllocateVirtualMemory(INVALID_HANDLE_VALUE, &raw_memory, 1, &allocate_size,
			                                         MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE) :
			                 0;
			if (!raw_memory || (vmres != 0)) {
				log_errorf(HASH_LUA, ERROR_OUT_OF_MEMORY,
				           STRING_CONST("Unable to allocate %" PRIsize " bytes of memory in low 32bit address space"),
				           nsize);
				return 0;
			}
#else
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#ifdef MAP_32BIT
			raw_memory = mmap(0, nsize, PROT_READ | PROT_WRITE,
			                  MAP_32BIT | MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED, -1, 0);
			if (raw_memory == MAP_FAILED) {
				raw_memory = 0;
			}
#endif
			// On MacOSX app needs to be linked with -pagezero_size 10000 -image_base 100000000 to
			// 1) Free up low 4Gb address range by reducing page zero size
			// 2) Move executable base address above 4Gb to free up more memory address space
#define MMAP_REGION_START ((uintptr_t)0x10000)
#define MMAP_REGION_END ((uintptr_t)0x80000000)
			static atomicptr_t baseaddr = (void*)MMAP_REGION_START;
			bool retried = false;
			while (!raw_memory) {
				raw_memory = mmap(atomic_load_ptr(&baseaddr, memory_order_acquire), nsize, PROT_READ | PROT_WRITE,
				                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED, -1, 0);
				if (((uintptr_t)raw_memory >= MMAP_REGION_START) &&
				    (uintptr_t)pointer_offset(raw_memory, nsize) < MMAP_REGION_END) {
					atomic_store_ptr(&baseaddr, pointer_offset(raw_memory, nsize), memory_order_release);
					break;
				}
				if (raw_memory && (raw_memory != MAP_FAILED)) {
					if (munmap(raw_memory, nsize) < 0)
						log_warn(HASH_MEMORY, WARNING_SYSTEM_CALL_FAIL,
						         STRING_CONST("Failed to munmap pages outside wanted 32-bit range"));
				}
				raw_memory = 0;
				if (retried)
					break;
				retried = true;
				atomic_store_ptr(&baseaddr, (void*)MMAP_REGION_START, memory_order_release);
			}
#endif
			if (block) {
				if (raw_memory)
					memcpy(raw_memory, block, (nsize < osize) ? nsize : osize);
#if FOUNDATION_PLATFORM_WINDOWS
				if (!NtFreeVirtualMemory)
					NtFreeVirtualMemory =
					    (NtFreeVirtualMemoryFn)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtFreeVirtualMemory");
				if (NtFreeVirtualMemory) {
					SIZE_T old_size = 0;
					NtFreeVirtualMemory(INVALID_HANDLE_VALUE, &block, &old_size, MEM_RELEASE);
				}
#else
				munmap(block, osize);
#endif
			}
			block = raw_memory;
		} else
#endif
		{
			if (!block)
				block = memory_allocate(HASH_LUA, nsize, 0, MEMORY_PERSISTENT);
			else
				block = memory_reallocate(block, nsize, 0, osize, MEMORY_PERSISTENT);
		}
		if (!block && env && ((lua_t*)env)->state)
			log_panicf(HASH_LUA, ERROR_OUT_OF_MEMORY,
			           STRING_CONST("Unable to allocate Lua memory (%" PRIsize " bytes)"), nsize);
	}
	return block;
}

static FOUNDATION_NOINLINE int
lua_panic(lua_State* state) {
	string_const_t errmsg = {0, 0};
	errmsg.str = lua_tolstring(state, -1, &errmsg.length);
	// FOUNDATION_ASSERT_FAILFORMAT("unprotected error in call to Lua API: %.*s", errmsg.length,
	//                             errmsg.str);
	log_errorf(HASH_LUA, ERROR_EXCEPTION, STRING_CONST("unprotected error in call to Lua API: %.*s"),
	           STRING_FORMAT(errmsg));
	return 0;
}

lua_t**
lua_instances(void) {
	return _lua_instances;
}

lua_t*
lua_allocate(void) {
	lua_t* env = lua_allocator(0, 0, 0, sizeof(lua_t));

	// Foundation allocators can meet demands of luajit on both 32 and 64 bit platforms
	lua_State* state = env ? lua_newstate(lua_allocator, env) : nullptr;
	if (!state) {
		log_error(HASH_LUA, ERROR_INTERNAL_FAILURE, STRING_CONST("Unable to allocate Lua state"));
		memory_deallocate(env);
		return 0;
	}

	lua_atpanic(state, lua_panic);

	// Disable automagic gc
	lua_gc(state, LUA_GCCOLLECT, 0);

	lua_pushlightuserdata(state, env);
	lua_setlglobal(state, "__environment", 13);

	env->state = state;
	env->calldepth = 0;

#if BUILD_ENABLE_LUA_THREAD_SAFE
	semaphore_initialize(&env->execution_right, 1);
	env->queue_head = 0;
	atomic_store32(&env->queue_tail, 0);
#endif

	int stacksize = lua_gettop(state);

	luaL_openlibs(state);

	lua_module_registry_initialize(state);

	lua_pop(state, lua_gettop(state) - stacksize);

	array_push(_lua_instances, env);

	return env;
}

void
lua_deallocate(lua_t* env) {
	if (!env)
		return;

	FOUNDATION_ASSERT(env->calldepth == 0);
	FOUNDATION_ASSERT(env->state);

	lua_gc(env->state, LUA_GCCOLLECT, 0);

	lua_module_registry_finalize(env->state);

	lua_close(env->state);

#if BUILD_ENABLE_LUA_THREAD_SAFE
	semaphore_finalize(&env->execution_right);
#endif

	for (size_t ienv = 0, esize = array_size(_lua_instances); ienv != esize; ++ienv) {
		if (_lua_instances[ienv] == env) {
			array_erase(_lua_instances, ienv);
			break;
		}
	}

	memory_deallocate(env);
}

string_const_t
lua_get_string(lua_t* env, const char* property, size_t length) {
	string_const_t value = {0, 0};
	lua_State* state = env->state;

#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, true))
		return value;
#endif

	int stacksize = lua_gettop(env->state);
	if (lua_do_get(env, property, length) == LUA_OK)
		value.str = lua_tolstring(state, -1, &value.length);

	lua_pop(state, lua_gettop(state) - stacksize);

#if BUILD_ENABLE_LUA_THREAD_SAFE
	lua_release_execution_right(env);
#endif

	return value;
}

int
lua_get_int(lua_t* env, const char* property, size_t length) {
	int value = 0;
	lua_State* state = env->state;

#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (!lua_acquire_execution_right(env, true))
		return value;
#endif

	int stacksize = lua_gettop(env->state);

	if (lua_do_get(env, property, length) == LUA_OK)
		value = (int)lua_tointeger(state, -1);

	lua_pop(state, lua_gettop(state) - stacksize);

#if BUILD_ENABLE_LUA_THREAD_SAFE
	lua_release_execution_right(env);
#endif

	return value;
}

static lua_result_t
lua_do_get(lua_t* env, const char* property, size_t length) {
	lua_State* state;
	lua_result_t result;
	size_t start, next;
	char buffer[BUILD_SIZE_LUA_NAME_MAXLENGTH];
	string_t part;

	state = env->state;
	result = LUA_OK;

	next = string_find(property, length, '.', 0);
	if (next != STRING_NPOS) {
		part = string_copy(buffer, sizeof(buffer), property, next);
		lua_getlglobal(state, part.str, part.length);
		if (lua_isnil(state, -1)) {
			log_errorf(HASH_LUA, ERROR_INVALID_VALUE, STRING_CONST("Invalid script get, '%.*s' is not set (%.*s)"),
			           STRING_FORMAT(part), (int)length, property);
			return LUA_ERROR;
		} else if (!lua_istable(state, -1)) {
			log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
			           STRING_CONST("Invalid script get, existing data '%.*s' in '%.*s' is not a table"),
			           STRING_FORMAT(part), (int)length, property);
			return LUA_ERROR;
		}
		// Top of stack is now table
		FOUNDATION_ASSERT(lua_istable(state, -1));
		++next;
		start = next;

		next = string_find(property, length, '.', next);
		while (next != STRING_NPOS) {
			part = string_copy(buffer, sizeof(buffer), property + start, next - start);
			lua_pushlstring(state, part.str, part.length);
			lua_gettable(state, -2);
			if (lua_isnil(state, -1)) {
				log_errorf(HASH_LUA, ERROR_INVALID_VALUE, STRING_CONST("Invalid script call, '%.*s' is not set (%.*s)"),
				           STRING_FORMAT(part), (int)next, property);
				return LUA_ERROR;
			} else if (!lua_istable(state, -1)) {
				log_errorf(HASH_LUA, ERROR_INVALID_VALUE,
				           STRING_CONST("Invalid script call, existing data '%.*s' in '%.*s' is not a table"),
				           STRING_FORMAT(part), (int)next, property);
				return LUA_ERROR;
			}
			// Top of stack is now table
			FOUNDATION_ASSERT(lua_istable(state, -1));

			++next;
			start = next;
			next = string_find(property, length, '.', next);
		}

		part = string_copy(buffer, sizeof(buffer), property + start, length - start);
		lua_pushlstring(state, part.str, part.length);
		lua_gettable(state, -2);
	} else {
		part = string_copy(buffer, sizeof(buffer), property, length);
		lua_getlglobal(state, part.str, part.length);
	}

	if (lua_isnil(state, -1)) {
		// Property does not exist in Lua context
		log_errorf(HASH_LUA, ERROR_INVALID_VALUE, STRING_CONST("Invalid script get, '%.*s' is not a property"),
		           (int)length, property);
		return LUA_ERROR;
	}

	return result;
}

void
lua_execute(lua_t* env, int gc_time, bool force) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if ((env->queue[env->queue_head].cmd == LUACMD_WAIT) && !gc_time)
		return;  // Nothing executable pending

	if (!lua_acquire_execution_right(env, force))
		return;

	lua_execute_pending(env);
#else
	FOUNDATION_UNUSED(force);
#endif

	if (gc_time)
		lua_run_gc(env, gc_time);

#if BUILD_ENABLE_LUA_THREAD_SAFE
	lua_release_execution_right(env);
#endif
}

void
lua_timed_gc(lua_t* env, int milliseconds) {
#if BUILD_ENABLE_LUA_THREAD_SAFE
	if (lua_acquire_execution_right(env, false)) {
		lua_run_gc(env, milliseconds > 0 ? milliseconds : 0);
		lua_release_execution_right(env);
	}
#else
	lua_run_gc(env, milliseconds > 0 ? milliseconds : 0);
#endif
}

lua_t*
lua_from_state(lua_State* state) {
	void* env;
	lua_getlglobal(state, "__environment", 13);
	env = lua_touserdata(state, -1);
	lua_pop(state, 1);
	return env;
}

lua_State*
lua_state(lua_t* env) {
	return env->state;
}

int
lua_arch_is_fr2(int arch) {
	switch (arch) {
		case ARCHITECTURE_X86:
		case ARCHITECTURE_PPC:
		case ARCHITECTURE_ARM5:
		case ARCHITECTURE_ARM6:
		case ARCHITECTURE_ARM7:
		case ARCHITECTURE_ARM8:
		case ARCHITECTURE_MIPS:
			return false;
		default:
			break;
	}
	return true;
}

extern int
lua_modulemap_initialize(void);

extern void
lua_modulemap_finalize(void);

extern int
lua_symbol_initialize(void);

extern void
lua_symbol_finalize(void);

static bool _module_initialized;

int
lua_module_initialize(const lua_config_t config) {
	FOUNDATION_UNUSED(config);

	if (_module_initialized)
		return 0;

	_module_initialized = true;

	memset(&_lua_config, 0, sizeof(_lua_config));

	if (lua_symbol_initialize() < 0)
		return -1;

	if (lua_modulemap_initialize() < 0)
		return -1;

	hashmap_t* symbol_map = lua_symbol_lookup_map();
	hashmap_insert(symbol_map, hash(STRING_CONST("lua_symbol_load_foundation")),
	               (void*)(uintptr_t)lua_symbol_load_foundation);
	hashmap_insert(symbol_map, hash(STRING_CONST("lua_symbol_load_network")),
	               (void*)(uintptr_t)lua_symbol_load_network);
	hashmap_insert(symbol_map, hash(STRING_CONST("lua_symbol_load_resource")),
	               (void*)(uintptr_t)lua_symbol_load_resource);
	hashmap_insert(symbol_map, hash(STRING_CONST("lua_symbol_load_window")), (void*)(uintptr_t)lua_symbol_load_window);

	lua_module_register(STRING_CONST("foundation"), LUA_FOUNDATION_UUID, lua_module_loader, lua_symbol_load_foundation);
	lua_module_register(STRING_CONST("network"), LUA_NETWORK_UUID, lua_module_loader, lua_symbol_load_network);
	lua_module_register(STRING_CONST("resource"), LUA_RESOURCE_UUID, lua_module_loader, lua_symbol_load_resource);
	lua_module_register(STRING_CONST("window"), LUA_WINDOW_UUID, lua_module_loader, lua_symbol_load_window);

	resource_import_register(lua_import);
	resource_compile_register(lua_compile);

#if FOUNDATION_SIZE_POINTER == 8
	if (!lua_is_fr2())
		log_info(HASH_LUA, STRING_CONST("Initialized LuaJIT in compatibility 32-bit mode"));
#endif

	return 0;
}

void
lua_module_finalize(void) {
	if (!_module_initialized)
		return;

	lua_modulemap_finalize();
	lua_symbol_finalize();

	array_deallocate(_lua_instances);

	_module_initialized = false;
}

bool
lua_module_is_initialized(void) {
	return _module_initialized;
}

lua_config_t
lua_module_config(void) {
	return _lua_config;
}

void
lua_module_parse_config(const char* path, size_t path_size, const char* buffer, size_t size, const json_token_t* tokens,
                        size_t num_tokens) {
	FOUNDATION_UNUSED(path);
	FOUNDATION_UNUSED(path_size);
	FOUNDATION_UNUSED(buffer);
	FOUNDATION_UNUSED(size);
	FOUNDATION_UNUSED(tokens);
	FOUNDATION_UNUSED(num_tokens);
}
