/* read.c  -  Lua library  -  MIT License  -  2013 Mattias Jansson / Rampant Pixels
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

#include <foundation/stream.h>


const char* lua_read_stream( lua_State* state, void* user_data, size_t* size )
{
	lua_readstream_t* read = (lua_readstream_t*)user_data;

	if( stream_eos( read->stream ) )
	{
		if( *size )
			*size = 0;
		return 0;
	}
	
	uint64_t num_read = stream_read( read->stream, read->chunk, 512 );

	if( size )
		*size = (size_t)num_read;

	return num_read ? read->chunk : 0;
}


const char* lua_read_chunked_buffer( lua_State* state, void* user_data, size_t* size )
{	
	lua_readbuffer_t* read = (lua_readbuffer_t*)user_data;

	if( read->offset >= read->size )
	{
		if( *size )
			*size = 0;
		return 0;
	}

	const void* current_chunk = pointer_offset_const( read->buffer, read->offset );
	unsigned int chunk_size = *(const unsigned int*)current_chunk;
	
	read->offset += chunk_size + sizeof( unsigned int );
	if( size )
		*size = (size_t)chunk_size;

	//log_debugf( "Read Lua chunk:\n%s", pointer_offset_const( current_chunk, sizeof( unsigned int ) ) );

	return pointer_offset_const( current_chunk, sizeof( unsigned int ) );
}


const char* lua_read_buffer( lua_State* state, void* user_data, size_t* size )
{	
	lua_readbuffer_t* read = (lua_readbuffer_t*)user_data;

	if( read->offset >= read->size )
	{
		if( *size )
			*size = 0;
		return 0;
	}

	const void* current_chunk = pointer_offset_const( read->buffer, read->offset );
	unsigned int chunk_size = read->size - read->offset;
	
	read->offset += chunk_size;
	if( size )
		*size = (size_t)chunk_size;

	return current_chunk;
}


const char* lua_read_string( lua_State* state, void* user_data, size_t* size )
{
	lua_readstring_t* read = (lua_readstring_t*)user_data;

	if( !read->size )
	{
		if( size )
			*size = 0;
		return 0;
	}

	if( size )
		*size = read->size;
	read->size = 0;

	return read->string;
}
