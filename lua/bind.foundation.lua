
local ffi = require( "ffi" )
local C = ffi.C

-- Basic system, build and architecture types

ffi.cdef[[
typedef float    float32_t;
typedef double   float64_t;
int              system_size_real( void );
int              system_size_pointer( void );
int              system_size_wchar( void );
]]

if C.system_size_real() > 4 then
      ffi.cdef( "typedef float64_t real;" );
else
      ffi.cdef( "typedef float32_t real;" );
end

if C.system_size_pointer() > 4 then
      ffi.cdef( "typedef uint64_t hashtable_int_t;" );
else
      ffi.cdef( "typedef uint32_t hashtable_int_t;" );
end

if C.system_size_wchar() > 2 then
      ffi.cdef( "typedef uint32_t wchar_t;" );
else
      ffi.cdef( "typedef uint16_t wchar_t;" );
end

-- Foundation types and functions

ffi.cdef[[

typedef struct stream_t        stream_t;
typedef struct event_t         event_t;
typedef struct event_block_t   event_block_t;
typedef struct event_stream_t  event_stream_t;
typedef struct blowfish_t      blowfish_t;
typedef struct md5_t           md5_t;
typedef struct process_t       process_t;
typedef struct ringbuffer_t    ringbuffer_t;
typedef struct bitbuffer_t     bitbuffer_t;
typedef struct stream_buffer_t stream_buffer_t;
typedef struct stream_pipe_t   stream_pipe_t;
typedef struct stream_ringbuffer_t stream_ringbuffer_t;
typedef struct hashmap_t       hashmap_t;
typedef struct hashtable_t     hashtable_t;
typedef struct hashtable32_t   hashtable32_t;
typedef struct hashtable64_t   hashtable64_t;
typedef struct mutex_t         mutex_t;
typedef struct objectmap_t     objectmap_t;
typedef struct radixsort_t     radixsort_t;
typedef struct regex_t         regex_t;
typedef struct regex_capture_t regex_capture_t;
typedef struct semaphore_t     semaphore_t;

typedef uint64_t       object_t;
typedef uint64_t       hash_t;
typedef uint64_t       tick_t;
typedef real           deltatime_t;
typedef uint16_t       radixsort_index_t;

typedef struct { uint64_t word[2]; } uint128_t;
typedef struct { uint64_t word[4]; } uint256_t;

typedef uint128_t      uuid_t;
typedef uint128_t      version_t;

typedef int         (* error_callback_fn )( int, int );
typedef int         (* assert_handler_fn )( uint64_t, const char*, const char*, int, const char* );
typedef void        (* log_callback_fn )( uint64_t, int, const char* );
typedef void        (* object_deallocate_fn )( object_t, void* );
typedef void        (* profile_write_fn )( void*, uint64_t );
typedef void        (* profile_read_fn )( void*, uint64_t );
typedef stream_t*   (* stream_open_fn )( const char*, unsigned int );
typedef void*       (* thread_fn )( object_t, void* );

void*                  array_allocate_pointer( unsigned int );
int                    array_size( const void* );
const void*            array_element_pointer( const void*, int );
void                   array_set_element_pointer( const void*, int, void* );
void                   array_deallocate( void* );

assert_handler_fn      assert_handler( void );
void                   assert_set_handler( assert_handler_fn );
int                    assert_report( uint64_t, const char*, const char*, int, const char* );

stream_t*              asset_stream_open( const char*, unsigned int );

unsigned int           base64_encode( const void*, char*, unsigned int, unsigned int );
unsigned int           base64_decode( const char*, void*, unsigned int, unsigned int );

bitbuffer_t*           bitbuffer_allocate_buffer( void*, unsigned int, bool );
bitbuffer_t*           bitbuffer_allocate_stream( stream_t* );
void                   bitbuffer_deallocate( bitbuffer_t* );
void                   bitbuffer_initialize_buffer( bitbuffer_t*, void*, unsigned int, bool );
void                   bitbuffer_initialize_stream( bitbuffer_t*, stream_t* );
void                   bitbuffer_finalize( bitbuffer_t* );
uint32_t               bitbuffer_read32( bitbuffer_t*, unsigned int );
uint64_t               bitbuffer_read64( bitbuffer_t*, unsigned int );
uint128_t              bitbuffer_read128( bitbuffer_t*, unsigned int );
float32_t              bitbuffer_read_float32( bitbuffer_t* );
float64_t              bitbuffer_read_float64( bitbuffer_t* );
void                   bitbuffer_write32( bitbuffer_t*, uint32_t, unsigned int );
void                   bitbuffer_write64( bitbuffer_t*, uint64_t, unsigned int );
void                   bitbuffer_write128( bitbuffer_t*, uint128_t, unsigned int );
void                   bitbuffer_write_float32( bitbuffer_t*, float32_t );
void                   bitbuffer_write_float64( bitbuffer_t*, float64_t );
void                   bitbuffer_align_read( bitbuffer_t*, bool );
void                   bitbuffer_align_write( bitbuffer_t*, bool );
void                   bitbuffer_discard_read( bitbuffer_t* );
void                   bitbuffer_discard_write( bitbuffer_t* );

uint16_t               byteorder_swap16( uint16_t );
uint32_t               byteorder_swap32( uint32_t );
uint64_t               byteorder_swap64( uint64_t );
void                   byteorder_swap( void*, const int );
uint16_t               byteorder_bigendian16( uint16_t );
uint32_t               byteorder_bigendian32( uint32_t );
uint64_t               byteorder_bigendian64( uint64_t );
void                   byteorder_bigendian( void*, const int );
uint16_t               byteorder_littleendian16( uint16_t );
uint32_t               byteorder_littleendian32( uint32_t );
uint64_t               byteorder_littleendian64( uint64_t );
void                   byteorder_littleendian( void*, const int );

blowfish_t*            blowfish_allocate( void );
void                   blowfish_deallocate( blowfish_t* blowfish );
void                   blowfish_initialize( blowfish_t* blowfish, const void* key, const unsigned int len );
void                   blowfish_finalize( blowfish_t* blowfish );
void                   blowfish_encrypt( const blowfish_t* blowfish, void* data, unsigned int, const int mode, const uint64_t vec );
void                   blowfish_decrypt( const blowfish_t* blowfish, void* data, unsigned int, const int mode, const uint64_t vec );

stream_t*              buffer_stream_allocate( void*, unsigned int, uint64_t, uint64_t, bool, bool );
void                   buffer_stream_initialize( stream_buffer_t*, void*, unsigned int, uint64_t, uint64_t, bool, bool );

bool                   config_bool( hash_t section, hash_t key );
int64_t                config_int( hash_t section, hash_t key );
real                   config_real( hash_t section, hash_t key );
const char*            config_string( hash_t section, hash_t key );
hash_t                 config_string_hash( hash_t section, hash_t key );
void                   config_set_bool( hash_t section, hash_t key, bool value );
void                   config_set_int( hash_t section, hash_t key, int64_t value );
void                   config_set_real( hash_t section, hash_t key, real value );
void                   config_set_string( hash_t section, hash_t key, const char* value );
void                   config_load( const char* name, hash_t filter_section, bool built_in, bool overwrite );
void                   config_parse( stream_t* stream, hash_t filter_section, bool overwrite );
void                   config_write( stream_t* stream, hash_t filter_section );
void                   config_parse_commandline( const char* const* cmdline, unsigned int num );

const char* const*     environment_command_line( void );
const char*            environment_executable_name( void );
const char*            environment_executable_directory( void );
const char*            environment_executable_path( void );
const char*            environment_initial_working_directory( void );
const char*            environment_current_working_directory( void );
void                   environment_set_current_working_directory( const char* );
const char*            environment_home_directory( void );
const char*            environment_temporary_directory( void );
const char*            environment_variable( const char* );

int                    error( void );
int                    error_report( int, int );
void                   error_context_push( const char*, const char* );
void                   error_context_pop( void );
error_callback_fn      error_callback( void );
void                   error_set_callback( error_callback_fn );

void                   event_post( event_stream_t*, uint8_t, uint8_t, uint16_t, object_t, const void*, tick_t );
event_t*               event_next( const event_block_t*, event_t* );
uint16_t               event_payload_size( const event_t* );
event_stream_t*        event_stream_allocate( unsigned int );
void                   event_stream_deallocate( event_stream_t* );
void                   event_stream_initialize( event_stream_t*, unsigned int );
void                   event_stream_finalize( event_stream_t* );
event_block_t*         event_stream_process( event_stream_t* );

stream_t*              fs_open_file( const char* path, unsigned int mode );
void                   fs_copy_file( const char* source, const char* dest );
bool                   fs_remove_file( const char* path );
bool                   fs_is_file( const char* path );
bool                   fs_make_directory( const char* path );
bool                   fs_remove_directory( const char* path );
bool                   fs_is_directory( const char* path );
uint64_t               fs_last_modified( const char* path );
void                   fs_touch( const char* path );
uint128_t              fs_md5( const char* path );
char**                 fs_matching_files( const char* path, const char* pattern, bool recurse );
char**                 fs_files( const char* path );
char**                 fs_subdirs( const char* path );
void                   fs_monitor( const char* path );
void                   fs_unmonitor( const char* path );
stream_t*              fs_temporary_file( void );
void                   fs_post_event( int id, const char* path, unsigned int pathlen );
event_stream_t*        fs_event_stream( void );

version_t              foundation_version( void );

hash_t                 hash( const char*, unsigned int );
const char*            hash_to_string( const hash_t );

hashmap_t*             hashmap_allocate( unsigned int, unsigned int );
void                   hashmap_deallocate( hashmap_t* );
void                   hashmap_initialize( hashmap_t*, unsigned int, unsigned int );
void                   hashmap_finalize( hashmap_t* );
void*                  hashmap_insert( hashmap_t*, hash_t, void* );
void*                  hashmap_erase( hashmap_t*, hash_t );
void*                  hashmap_lookup( hashmap_t*, hash_t );
bool                   hashmap_has_key( hashmap_t*, hash_t );
unsigned int           hashmap_size( hashmap_t* );
void                   hashmap_clear( hashmap_t* );

hashtable_t*           hashtable_allocate( unsigned int );
void                   hashtable_deallocate( hashtable_t* );
void                   hashtable_initialize( hashtable_t*, unsigned int );
void                   hashtable_finalize( hashtable_t* );
void                   hashtable_set( hashtable_t*, hashtable_int_t, hashtable_int_t );
void                   hashtable_erase( hashtable_t*, hashtable_int_t );
hashtable_int_t        hashtable_get( hashtable_t*, hashtable_int_t );
unsigned int           hashtable_size( hashtable_t* );
void                   hashtable_clear( hashtable_t* );
hashtable32_t*         hashtable32_allocate( unsigned int );
void                   hashtable32_deallocate( hashtable32_t* );
void                   hashtable32_initialize( hashtable32_t*, unsigned int );
void                   hashtable32_finalize( hashtable32_t* );
void                   hashtable32_set( hashtable32_t*, uint32_t, uint32_t );
void                   hashtable32_erase( hashtable32_t*, uint32_t );
uint32_t               hashtable32_get( hashtable32_t*, uint32_t );
unsigned int           hashtable32_size( hashtable32_t* );
void                   hashtable32_clear( hashtable32_t* );
hashtable64_t*         hashtable64_allocate( unsigned int );
void                   hashtable64_deallocate( hashtable64_t* );
void                   hashtable64_initialize( hashtable64_t*, unsigned int );
void                   hashtable64_finalize( hashtable64_t* );
void                   hashtable64_set( hashtable64_t*, uint64_t, uint64_t );
void                   hashtable64_erase( hashtable64_t*, uint64_t );
uint64_t               hashtable64_get( hashtable64_t*, uint64_t );
unsigned int           hashtable64_size( hashtable64_t* );
void                   hashtable64_clear( hashtable64_t* );

object_t               library_load( const char* );
object_t               library_ref( object_t );
void                   library_unload( object_t );
void*                  library_symbol( object_t, const char* );
const char*            library_name( object_t );
bool                   library_valid( object_t );

void                   log_debugf( uint64_t, const char*, ... );
void                   log_infof( uint64_t, const char*, ... );
void                   log_warnf( uint64_t, int, const char*, ... );
void                   log_errorf( uint64_t, int, const char*, ... );
void                   log_panicf( uint64_t, int, const char*, ... );
void                   log_enable_stdout( bool );
void                   log_enable_prefix( bool );
void                   log_set_suppress( uint64_t, int );
int                    log_suppress( uint64_t );
void                   log_suppress_clear( void );
void                   log_error_context( uint64_t, int );
log_callback_fn        log_callback( void );
void                   log_set_callback( log_callback_fn );

md5_t*                 md5_allocate( void );
void                   md5_deallocate( md5_t* );
void                   md5_initialize( md5_t* );
void                   md5_finalize( md5_t* );
md5_t*                 md5_digest( md5_t*, const char* );
md5_t*                 md5_digest_raw( md5_t*, const void*, size_t );
void                   md5_digest_finalize( md5_t* );
char*                  md5_get_digest( const md5_t* );
uint128_t              md5_get_digest_raw( const md5_t* );

void*                  memory_allocate( uint64_t, uint64_t, unsigned int, int );
void*                  memory_reallocate( void*, uint64_t, unsigned int, uint64_t );
void                   memory_deallocate( void* );
void                   memory_context_push( uint64_t );
void                   memory_context_pop( void );
uint64_t               memory_context( void );

mutex_t*               mutex_allocate( const char* );
void                   mutex_deallocate( mutex_t* );
const char*            mutex_name( mutex_t* );
bool                   mutex_try_lock( mutex_t* );
bool                   mutex_lock( mutex_t* );
bool                   mutex_unlock( mutex_t* );
bool                   mutex_wait( mutex_t*, unsigned int );
void                   mutex_signal( mutex_t* );

objectmap_t*           objectmap_allocate( unsigned int );
void                   objectmap_deallocate( objectmap_t* );
void                   objectmap_initialize( objectmap_t*, unsigned int );
void                   objectmap_finalize( objectmap_t* );
unsigned int           objectmap_size( const objectmap_t* );
object_t               objectmap_reserve( objectmap_t* );
void                   objectmap_free( objectmap_t*, object_t );
void                   objectmap_set( objectmap_t*, object_t, void* );
void*                  objectmap_raw_lookup( const objectmap_t*, unsigned int );
void*                  objectmap_lookup_ref( const objectmap_t*, object_t );
bool                   objectmap_lookup_unref( const objectmap_t*, object_t, object_deallocate_fn );
void*                  objectmap_lookup( const objectmap_t*, object_t );

char*                  path_base_file_name( const char* path );
char*                  path_base_file_name_with_path( const char* path );
char*                  path_file_extension( const char* path );
char*                  path_file_name( const char* path );
char*                  path_directory_name( const char* path );
char*                  path_subdirectory_name( const char* path, const char* root );
char*                  path_protocol( const char* uri );

char*                  path_merge( const char*, const char* );
char*                  path_append( char*, const char* );
char*                  path_prepend( char*, const char* );
char*                  path_clean( char*, bool );
bool                   path_is_absolute( const char* );
char*                  path_make_absolute( const char* );
char*                  path_make_temporary( void );

stream_t*              pipe_allocate( void );
void                   pipe_initialize( stream_pipe_t* );
void                   pipe_close_read( stream_t* );
void                   pipe_close_write( stream_t* );

process_t*             process_allocate( void );
void                   process_deallocate( process_t* );
void                   process_initialize( process_t* );
void                   process_finalize( process_t* );
void                   process_set_working_directory( process_t*, const char* );
void                   process_set_executable_path( process_t*, const char* );
void                   process_set_arguments( process_t*, const char**, unsigned int );
void                   process_set_flags( process_t*, unsigned int );
void                   process_set_verb( process_t*, const char* );
void                   process_set_exit_code( int );
int                    process_spawn( process_t* );
stream_t*              process_stdout( process_t* );
stream_t*              process_stdin( process_t* );
int                    process_wait( process_t* );
int                    process_exit_code( void );
void                   process_exit( int );

void                   profile_initialize( const char*, void*, uint64_t );
void                   profile_shutdown( void );
void                   profile_enable( bool );
void                   profile_set_output( profile_write_fn );
void                   profile_set_output_wait( int );
void                   profile_end_frame( uint64_t );
void                   profile_begin_block( const char* );
void                   profile_update_block( void );
void                   profile_end_block( void );
void                   profile_log( const char* );
void                   profile_trylock( const char* );
void                   profile_lock( const char* );
void                   profile_unlock( const char* );
void                   profile_wait( const char* );
void                   profile_signal( const char* );

radixsort_t*           radixsort_allocate( int, radixsort_index_t );
void                   radixsort_deallocate( radixsort_t* );
void                   radixsort_initialize( radixsort_t*, int, radixsort_index_t );
void                   radixsort_finalize( radixsort_t* );
const radixsort_index_t* radixsort( radixsort_t*, const void*, radixsort_index_t );

uint32_t               random32( void );
uint32_t               random32_range( uint32_t, uint32_t );
uint64_t               random64( void );
uint64_t               random64_range( uint64_t, uint64_t );
real                   random_normalized( void );
real                   random_range( real, real );
int32_t                random32_gaussian_range( int32_t, int32_t );
real                   random_gaussian_range( real, real );
int32_t                random32_triangle_range( int32_t, int32_t );
real                   random_triangle_range( real, real );
uint32_t               random32_weighted( uint32_t, const real* );

regex_t*               regex_compile( const char* );
bool                   regex_match( regex_t*, const char*, int, regex_capture_t*, int );
void                   regex_deallocate( regex_t* );

ringbuffer_t*          ringbuffer_allocate( unsigned int );
void                   ringbuffer_deallocate( ringbuffer_t* );
void                   ringbuffer_initialize( ringbuffer_t*, unsigned int );
void                   ringbuffer_finalize( ringbuffer_t* );
unsigned int           ringbuffer_size( ringbuffer_t* );
void                   ringbuffer_reset( ringbuffer_t* );
unsigned int           ringbuffer_read( ringbuffer_t*, void*, unsigned int );
unsigned int           ringbuffer_write( ringbuffer_t*, const void*, unsigned int );
uint64_t               ringbuffer_total_read( ringbuffer_t* );
uint64_t               ringbuffer_total_written( ringbuffer_t* );
stream_t*              ringbuffer_stream_allocate( unsigned int, uint64_t );
void                   ringbuffer_stream_initialize( stream_ringbuffer_t*, unsigned int, uint64_t );

void                   semaphore_initialize( semaphore_t*, unsigned int );
void                   semaphore_initialize_named( semaphore_t*, const char*, unsigned int );
void                   semaphore_finalize( semaphore_t* );
bool                   semaphore_wait( semaphore_t* );
bool                   semaphore_try_wait( semaphore_t*, int );
void                   semaphore_post( semaphore_t* );

unsigned int           stacktrace_capture( void**, unsigned int, unsigned int );
char*                  stacktrace_resolve( void**, unsigned int, unsigned int );

stream_t*              stream_open( const char*, unsigned int );
stream_t*              stream_clone( stream_t* );
void                   stream_deallocate( stream_t* );
void                   stream_initialize( stream_t*, int );
void                   stream_finalize( stream_t* );
int64_t                stream_tell( stream_t* );
void                   stream_seek( stream_t*, int64_t, int );
bool                   stream_eos( stream_t* );
uint64_t               stream_size( stream_t* );
void                   stream_set_byteorder( stream_t*, int );
void                   stream_set_binary( stream_t*, bool );
void                   stream_determine_binary_mode( stream_t*, unsigned int );
bool                   stream_is_binary( stream_t* );
bool                   stream_is_sequential( stream_t* );
bool                   stream_is_reliable( stream_t* );
bool                   stream_is_inorder( stream_t* );
bool                   stream_is_swapped( stream_t* );
int                    stream_byteorder( stream_t* );
const char*            stream_path( const stream_t* );
uint64_t               stream_last_modified( stream_t* );
uint64_t               stream_read( stream_t*, void*, uint64_t );
uint64_t               stream_read_line_buffer( stream_t*, char*, unsigned int, char );
char*                  stream_read_line( stream_t*, char );
bool                   stream_read_bool( stream_t* );
int8_t                 stream_read_int8( stream_t* );
uint8_t                stream_read_uint8( stream_t* );
int16_t                stream_read_int16( stream_t* );
uint16_t               stream_read_uint16( stream_t* );
int32_t                stream_read_int32( stream_t* );
uint32_t               stream_read_uint32( stream_t* );
int64_t                stream_read_int64( stream_t* );
uint64_t               stream_read_uint64( stream_t* );
float32_t              stream_read_float32( stream_t* );
float64_t              stream_read_float64( stream_t* );
char*                  stream_read_string( stream_t* );
uint64_t               stream_read_string_buffer( stream_t*, char*, uint64_t );
uint64_t               stream_write( stream_t*, const void*, uint64_t );
void                   stream_write_bool( stream_t*, bool );
void                   stream_write_int8( stream_t*, int8_t );
void                   stream_write_uint8( stream_t*, uint8_t );
void                   stream_write_int16( stream_t*, int16_t );
void                   stream_write_uint16( stream_t*, uint16_t );
void                   stream_write_int32( stream_t*, int32_t );
void                   stream_write_uint32( stream_t*, uint32_t );
void                   stream_write_int64( stream_t*, int64_t );
void                   stream_write_uint64( stream_t*, uint64_t );
void                   stream_write_float32( stream_t*, float32_t );
void                   stream_write_float64( stream_t*, float64_t );
void                   stream_write_string( stream_t*, const char* );
void                   stream_write_endl( stream_t* );
void                   stream_write_format( stream_t*, const char*, ... );
void                   stream_buffer_read( stream_t* );
unsigned int           stream_available_read( stream_t* );
uint128_t              stream_md5( stream_t* );
void                   stream_truncate( stream_t*, uint64_t );
void                   stream_flush( stream_t* );
stream_t*              stream_open_stdout( void );
stream_t*              stream_open_stderr( void );
stream_t*              stream_open_stdin( void );
void                   stream_set_protocol_handler( const char*, stream_open_fn );
stream_open_fn         stream_protocol_handler( const char*, unsigned int );

char*                  string_allocate( unsigned int );
void                   string_deallocate( char* );
char*                  string_clone( const char* );
char*                  string_format( const char*, ... );
char*                  string_format_buffer( char*, unsigned int, const char*, ... );
unsigned int           string_length( const char* );
unsigned int           string_glyphs( const char* );
hash_t                 string_hash( const char* );
char*                  string_resize( char*, unsigned int, char );
void                   string_copy( char* dst, const char*, unsigned int );
char*                  string_strip( char*, const char* );
char*                  string_strip_substr( char*, const char*, unsigned int );
char*                  string_replace( char*, const char*, const char*, bool );
char*                  string_append( char*, const char* );
char*                  string_prepend( char*, const char* );
char*                  string_concat( const char*, const char* );
void                   string_split( const char*, const char*, char**, char**, bool );
char*                  string_substr( const char*, unsigned int, unsigned int );
unsigned int           string_find( const char*, char, unsigned int );
unsigned int           string_find_string( const char*, const char*, unsigned int );
unsigned int           string_rfind( const char*, char, unsigned int );
unsigned int           string_rfind_string( const char*, const char*, unsigned int );
unsigned int           string_find_first_of( const char*, const char*, unsigned int );
unsigned int           string_find_last_of( const char*, const char*, unsigned int );
unsigned int           string_find_first_not_of( const char*, const char*, unsigned int );
unsigned int           string_find_last_not_of( const char*, const char*, unsigned int );
unsigned int           string_rfind_first_of( const char*, const char*, unsigned int );
unsigned int           string_rfind_first_not_of( const char*, const char*, unsigned int );
bool                   string_ends_with( const char*, const char* );
bool                   string_equal( const char*, const char* );
bool                   string_equal_substr( const char*, const char*, unsigned int );
bool                   string_match_pattern( const char*, const char* );
char**                 string_explode( const char*, const char*, bool );
char*                  string_merge( const char* const*, unsigned int, const char* );
uint32_t               string_glyph( const char*, unsigned int, unsigned int* );
int                    string_array_find( const char* const*, const char*, unsigned int );
void                   string_array_deallocate_elements( char** );
void                   string_array_deallocate( char** );
wchar_t*               wstring_allocate_from_string( const char*, unsigned int );
void                   wstring_from_string( wchar_t*, const char*, unsigned int );
void                   wstring_deallocate( wchar_t* );
unsigned int           wstring_length( const wchar_t* );
bool                   wstring_equal( const wchar_t*, const wchar_t* );
char*                  string_allocate_from_wstring( const wchar_t*, unsigned int );
char*                  string_allocate_from_utf16( const uint16_t*, unsigned int );
char*                  string_allocate_from_utf32( const uint32_t*, unsigned int );
void                   string_convert_utf16( char*, const uint16_t*, unsigned int, unsigned int );
void                   string_convert_utf32( char*, const uint32_t*, unsigned int, unsigned int );
char*                  string_from_int( int64_t, unsigned int, char );
char*                  string_from_uint( uint64_t, bool, unsigned int, char );
char*                  string_from_uint128( const uint128_t );
char*                  string_from_real( real, unsigned int, unsigned int, char );
char*                  string_from_time( uint64_t );
char*                  string_from_uuid( const uuid_t );
char*                  string_from_version( const version_t );
char*                  string_from_int_buffer( char*, int64_t, unsigned int, char );
char*                  string_from_uint_buffer( char*, uint64_t, bool, unsigned int, char );
char*                  string_from_uint128_buffer( char*, const uint128_t );
char*                  string_from_real_buffer( char*, real, unsigned int precision, unsigned int, char );
char*                  string_from_time_buffer( char*, uint64_t );
char*                  string_from_uuid_buffer( char*, const uuid_t );
char*                  string_from_version_buffer( char*, const version_t );
const char*            string_from_int_static( int64_t, unsigned int, char );
const char*            string_from_uint_static( uint64_t, bool, unsigned int, char );
const char*            string_from_uint128_static( const uint128_t );
const char*            string_from_real_static( real, unsigned int precision, unsigned int, char );
const char*            string_from_time_static( uint64_t );
const char*            string_from_uuid_static( const uuid_t );
const char*            string_from_version_static( const version_t );
int                    string_to_int( const char* );
unsigned int           string_to_uint( const char*, bool );
int64_t                string_to_int64( const char* );
uint64_t               string_to_uint64( const char*, bool );
uint128_t              string_to_uint128( const char* );
float32_t              string_to_float32( const char* );
float64_t              string_to_float64( const char* );
real                   string_to_real( const char* );
uuid_t                 string_to_uuid( const char* );
version_t              string_to_version( const char* );

int                    system_error( void );
void                   system_error_reset( void );
const char*            system_error_message( int );
int                    system_platform( void );
int                    system_architecture( void );
int                    system_byteorder( void );
unsigned int           system_hardware_threads( void );
const char*            system_hostname( void );
uint64_t               system_hostid( void );
const char*            system_username( void );
bool                   system_debugger_attached( void );
void                   system_pause( void );
uint16_t               system_language( void );
uint16_t               system_country( void );
uint32_t               system_locale( void );
const char*            system_locale_string( void );
void                   system_set_device_orientation( int );
int                    system_device_orientation( void );
void                   system_process_events( void );
bool                   system_message_box( const char*, const char*, bool );
event_stream_t*        system_event_stream( void );
void                   system_post_event( int );

object_t               thread_create( thread_fn, const char*, int, unsigned int );
object_t               thread_ref( object_t );
void                   thread_destroy( object_t );
bool                   thread_start( object_t, void* );
void                   thread_terminate( object_t );
bool                   thread_is_started( object_t );
bool                   thread_is_running( object_t );
bool                   thread_is_thread( object_t );
bool                   thread_is_main( void );
bool                   thread_should_terminate( object_t );
void                   thread_set_main( void );
void                   thread_set_name( const char* );
void                   thread_set_hardware( uint64_t );
void*                  thread_result( object_t );
object_t               thread_self( void );
const char*            thread_name( void );
uint64_t               thread_id( void );
unsigned int           thread_hardware( void );
void                   thread_sleep( int );
void                   thread_yield( void );
void                   thread_finalize( void );
void*                  thread_attach_jvm( void );
void                   thread_detach_jvm( void );

tick_t                 time_current( void );
tick_t                 time_diff( const tick_t from, const tick_t to );
deltatime_t            time_elapsed( const tick_t since );
tick_t                 time_elapsed_ticks( const tick_t since );
tick_t                 time_ticks_per_second( void );
deltatime_t            time_ticks_to_seconds( const tick_t dt );
tick_t                 time_startup( void );
tick_t                 time_system( void );

uuid_t                 uuid_generate_time( void );
uuid_t                 uuid_generate_name( const uuid_t, const char* );
uuid_t                 uuid_generate_random( void );
bool                   uuid_equal( const uuid_t, const uuid_t );
uuid_t                 uuid_null( void );
bool                   uuid_is_null( const uuid_t );
uuid_t                 uuid_dns( void );

]]

-- Helper functions

function hash( str ) return C.hash( str, C.string_length( str ) ) end

function string_array_to_table( arr )
      local tab = {}
      local arr_size = C.array_size( arr )
      for i = 1, arr_size do
            tab[i] = ffi.string( C.array_element_pointer( arr, i-1 ) )
      end
      return tab
end

function string_table_to_array( tab )
      local num = 0
      while tab[num+1] ~= nil do num = num + 1 end
      local arr = C.array_allocate_pointer( num )
      for i = 0, num do
      	  C.array_set_element_pointer( arr, i, tab[i+1] )
      end
      return arr, num
end

-- Constants

PLATFORM_WINDOWS = 1
PLATFORM_LINUX = 2
PLATFORM_MACOSX = 3
PLATFORM_IOS = 4
PLATFORM_ANDROID = 5
PLATFORM_RASPBERRYPI = 6
PLATFORM_PNACL = 7
PLATFORM_BSD = 8
PLATFORM_TIZEN = 9

ARCHITECTURE_X86 = 0
ARCHITECTURE_X86_64 = 1
ARCHITECTURE_PPC = 2
ARCHITECTURE_PPC_64 = 3
ARCHITECTURE_ARM5 = 4
ARCHITECTURE_ARM6 = 5
ARCHITECTURE_ARM7 = 6
ARCHITECTURE_ARM8 = 7
ARCHITECTURE_ARM8_64 = 8
ARCHITECTURE_MIPS = 9
ARCHITECTURE_MIPS_64 = 10
ARCHITECTURE_GENERIC = 11

BYTEORDER_LITTLEENDIAN = 0
BYTEORDER_BIGENDIAN = 1

FOUNDATIONEVENT_START = 1
FOUNDATIONEVENT_TERMINATE = 2
FOUNDATIONEVENT_PAUSE = 3
FOUNDATIONEVENT_RESUME = 4
FOUNDATIONEVENT_FOCUS_GAIN = 5
FOUNDATIONEVENT_FOCUS_LOST = 6
FOUNDATIONEVENT_FILE_CREATED = 7
FOUNDATIONEVENT_FILE_DELETED = 8
FOUNDATIONEVENT_FILE_MODIFIED = 9
FOUNDATIONEVENT_LOW_MEMORY_WARNING = 10
FOUNDATIONEVENT_DEVICE_ORIENTATION = 11

EVENTFLAG_DELAY = 1

ERRORLEVEL_NONE = 0
ERRORLEVEL_DEBUG = 1
ERRORLEVEL_INFO = 2
ERRORLEVEL_WARNING = 3
ERRORLEVEL_ERROR = 4
ERRORLEVEL_PANIC = 5

ERROR_NONE = 0
ERROR_INVALID_VALUE = 1
ERROR_UNSUPPORTED = 2
ERROR_NOT_IMPLEMENTED = 3
ERROR_OUT_OF_MEMORY = 4
ERROR_MEMORY_LEAK = 5
ERROR_MEMORY_ALIGNMENT = 6
ERROR_INTERNAL_FAILURE = 7
ERROR_ACCESS_DENIED = 8
ERROR_EXCEPTION = 9
ERROR_SYSTEM_CALL_FAIL = 10
ERROR_UNKNOWN_TYPE = 11
ERROR_UNKNOWN_RESOURCE = 12
ERROR_DEPRECATED = 13
ERROR_ASSERT = 14
ERROR_SCRIPT = 15

WARNING_PERFORMANCE = 0
WARNING_DEPRECATED = 1
WARNING_BAD_DATA = 2
WARNING_MEMORY = 3
WARNING_UNSUPPORTED = 4
WARNING_SUSPICIOUS = 5
WARNING_SYSTEM_CALL_FAIL = 6
WARNING_DEADLOCK = 7
WARNING_SCRIPT = 8
WARNING_RESOURCE = 9

STREAM_IN = 0x0001
STREAM_OUT = 0x0002
STREAM_TRUNCATE = 0x0010
STREAM_CREATE = 0x0020
STREAM_ATEND = 0x0040
STREAM_BINARY = 0x0100
STREAM_SYNC = 0x0200

STREAM_SEEK_BEGIN = 0
STREAM_SEEK_CURRENT = 1
STREAM_SEEK_END = 2

BLOWFISH_ECB = 0
BLOWFISH_CBC = 1
BLOWFISH_CFB = 2
BLOWFISH_OFB = 3

RADIXSORT_INT32 = 0
RADIXSORT_UINT32 = 1
RADIXSORT_INT64 = 2
RADIXSORT_UINT64 = 3
RADIXSORT_FLOAT32 = 4
RADIXSORT_FLOAT64 = 5

DEVICEORIENTATION_UNKNOWN = 0
DEVICEORIENTATION_PORTRAIT = 1
DEVICEORIENTATION_PORTRAIT_FLIPPED = 2
DEVICEORIENTATION_LANDSCAPE_CCW = 3
DEVICEORIENTATION_LANDSCAPE_CW = 4
DEVICEORIENTATION_FACEUP = 5
DEVICEORIENTATION_FACEDOWN = 6

PROCESS_ATTACHED = 0
PROCESS_DETACHED = 0x01
PROCESS_CONSOLE = 0x02
PROCESS_STDSTREAMS = 0x04
PROCESS_WINDOWS_USE_SHELLEXECUTE = 0x08
PROCESS_MACOSX_USE_OPENAPPLICATION = 0x10
PROCESS_INVALID_ARGS = 0x7FFFFFF0
PROCESS_TERMINATED_SIGNAL = 0x7FFFFFF1
PROCESS_WAIT_INTERRUPTED = 0x7FFFFFF2
PROCESS_WAIT_FAILED = 0x7FFFFFF3
PROCESS_STILL_ACTIVE = 0x7FFFFFFF

STREAMTYPE_INVALID = 0
STREAMTYPE_MEMORY = 1
STREAMTYPE_FILE = 2
STREAMTYPE_SOCKET = 3
STREAMTYPE_RINGBUFFER = 4
STREAMTYPE_ASSET = 5
STREAMTYPE_PIPE = 6
STREAMTYPE_STDSTREAM = 7

THREAD_PRIORITY_LOW = 0
THREAD_PRIORITY_BELOWNORMAL = 1
THREAD_PRIORITY_NORMAL = 2
THREAD_PRIORITY_ABOVENORMAL = 3
THREAD_PRIORITY_HIGHEST = 4
THREAD_PRIORITY_TIMECRITICAL = 5

MEMORY_PERSISTENT          = 0x0000
MEMORY_TEMPORARY           = 0x0001
MEMORY_THREAD              = 0x0002
MEMORY_32BIT_ADDRESS       = 0x0004
MEMORY_ZERO_INITIALIZED    = 0x0008

-- Context hash value is hash of "lua", since luajit parser does not handle
-- full 64bit hex numbers (numbers > 0xc000000000000000 are truncated)
HASH_LUA = hash( "lua" )

-- Convenience wrappers

function log_debug( message ) C.log_debugf( HASH_LUA, "%s", message ) end
function log_info( message ) C.log_infof( HASH_LUA, "%s", message ) end
function log_warn( message ) C.log_warnf( HASH_LUA, 8, "%s", message ) end -- 8 = WARNING_SCRIPT
function log_error( message ) C.log_errorf( HASH_LUA, 15, "%s", message ) end -- 15 = ERROR_SCRIPT
function log_panic( message ) C.log_panicf( HASH_LUA, 15, "%s", message ) end -- 15 = ERROR_SCRIPT

function string_explode( str, delimiters, allow_empty ) local arr = C.string_explode( str, delimiters, allow_empty ); local tab = string_array_to_table( arr ); C.array_deallocate( arr ); return tab end
function string_merge( tab, delimiter ) local arr, num = string_table_to_array( tab ); local cstr = C.string_merge( arr, num, delimiter ); local str = ffi.string( cstr ); C.string_deallocate( cstr ); C.array_deallocate( arr ); return str end

function environment_command_line() local arr = C.environment_command_line(); return string_array_to_table( arr ) end

function stream_read_line( stream, delimiter ) local line = C.stream_read_line( stream, delimiter ); local str = ffi.string( line ); string.deallocate( line ); return str end
function stream_read_string( stream ) local line = C.stream_read_string( stream ); local str = ffi.string( line ); string.deallocate( line ); return str end

function fs_matching_files( path, pattern, recurse ) local arr = C.fs_matching_files( path, pattern, recurse ); local table = string_array_to_table( arr ); C.array_deallocate( arr ); return table end
function fs_files( path ) local arr = C.fs_files( path ); local table = string_array_to_table( arr ); C.array_deallocate( arr ); return table end
function fs_subdirs( path ) local arr = C.fs_subdirs( path ); local table = string_array_to_table( arr ); C.array_deallocate( arr ); return table end

function process_set_arguments( proc, args ) local arr, num = string_table_to_array( args ); C.process_set_arguments( proc, arr, num ); C.array_deallocate( arr ); end
