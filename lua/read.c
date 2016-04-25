/* read.c  -  Lua library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#include <lua/lua.h>

#include <foundation/stream.h>

const char*
lua_read_stream(lua_State* state, void* user_data, size_t* size) {
	lua_readstream_t* read = (lua_readstream_t*)user_data;

	FOUNDATION_UNUSED(state);

	if (stream_eos(read->stream)) {
		if (*size)
			*size = 0;
		return 0;
	}

	uint64_t num_read = stream_read(read->stream, read->chunk, 512);

	if (size)
		*size = (size_t)num_read;

	return num_read ? read->chunk : 0;
}

const char*
lua_read_chunked_buffer(lua_State* state, void* user_data, size_t* size) {
	lua_readbuffer_t* read = (lua_readbuffer_t*)user_data;

	FOUNDATION_UNUSED(state);

	if (read->offset >= read->size) {
		if (*size)
			*size = 0;
		return 0;
	}

	const void* current_chunk = pointer_offset_const(read->buffer, read->offset);
	unsigned int chunk_size = *(const unsigned int*)current_chunk;

	read->offset += chunk_size + sizeof(unsigned int);
	if (size)
		*size = (size_t)chunk_size;

	//log_debugf(HASH_LUA, STRING_CONST("Read Lua chunk:\n%s"), pointer_offset_const(current_chunk, sizeof(unsigned int)));

	return pointer_offset_const(current_chunk, sizeof(unsigned int));
}

const char*
lua_read_buffer(lua_State* state, void* user_data, size_t* size) {
	lua_readbuffer_t* read = (lua_readbuffer_t*)user_data;

	FOUNDATION_UNUSED(state);

	if (read->offset >= read->size) {
		if (*size)
			*size = 0;
		return 0;
	}

	const void* current_chunk = pointer_offset_const(read->buffer, read->offset);
	size_t chunk_size = (read->size - read->offset);

	read->offset += chunk_size;
	if (size)
		*size = chunk_size;

	return current_chunk;
}

const char*
lua_read_string(lua_State* state, void* user_data, size_t* size) {
	lua_readstring_t* read = (lua_readstring_t*)user_data;

	FOUNDATION_UNUSED(state);

	if (!read->size) {
		if (size)
			*size = 0;
		return 0;
	}

	if (size)
		*size = read->size;
	read->size = 0;

	return read->string;
}
