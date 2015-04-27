
local ffi = require( "ffi" )
local C = ffi.C

ffi.cdef[[

typedef struct stream_t        stream_t;
typedef struct event_t         event_t;
typedef struct event_block_t   event_block_t;
typedef struct event_stream_t  event_stream_t;
typedef struct blowfish_t      blowfish_t;
typedef struct md5_t           md5_t;
typedef struct process_t       process_t;
typedef struct ringbuffer_t    ringbuffer_t;

typedef float          float32_t;
typedef double         float64_t;
typedef float32_t      real; // Change this if compiling foundation with 64bit reals

typedef uint64_t       object_t;
typedef uint64_t       hash_t;
typedef uint64_t       tick_t;
typedef real           deltatime_t;

typedef struct { uint64_t word[2]; } uint128_t;
typedef struct { uint64_t word[4]; } uint256_t;

typedef uint128_t      uuid_t;

typedef int         (* error_callback_fn )( int, int );
typedef int         (* assert_handler_fn )( uint64_t, const char*, const char*, int, const char* );
typedef void        (* log_callback_fn )( uint64_t, int, const char* );

hash_t                 hash( const char*, unsigned int );
const char*            hash_to_string( const hash_t );

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

int                    error( void );
int                    error_report( int, int );
void                   error_context_push( const char*, const char* );
void                   error_context_pop( void );
error_callback_fn      error_callback( void );
void                   error_set_callback( error_callback_fn );

void                   string_deallocate( char* );
unsigned int           string_length( const char* );
unsigned int           string_glyphs( const char* );
char**                 string_explode( const char* str, const char* delimiters, bool allow_empty );
char*                  string_merge( const char* const* array, unsigned int array_size, const char* delimiter );
const char*            string_from_uint128_static( const uint128_t val );
const char*            string_from_time_static( uint64_t time );
const char*            string_from_uuid_static( const uuid_t uuid );
uint64_t               string_to_uint64( const char* val, bool hex );
uint128_t              string_to_uint128( const char* val );
uuid_t                 string_to_uuid( const char* str );
bool                   string_is_valid_email_address( const char* address );

void*                  array_allocate_pointer( unsigned int );
int                    array_size( const void* );
const void*            array_element_pointer( const void*, int );
void                   array_set_element_pointer( const void*, int, void* );
void                   array_deallocate( void* );

const char* const*     environment_command_line( void );
const char*            environment_executable_name( void );
const char*            environment_executable_directory( void );
const char*            environment_initial_working_directory( void );
const char*            environment_current_working_directory( void );
void                   environment_set_current_working_directory( const char* );
const char*            environment_home_directory( void );
const char*            environment_temporary_directory( void );
const char*            environment_variable( const char* );

stream_t*              stream_open( const char*, unsigned int );
stream_t*              stream_clone( stream_t* );
void                   stream_deallocate( stream_t* );
int64_t                stream_tell( stream_t* );
void                   stream_seek( stream_t*, int64_t, int );
bool                   stream_eos( stream_t* );
void                   stream_set_byteorder( stream_t*, int );
void                   stream_set_binary( stream_t*, bool );
bool                   stream_is_sequential( stream_t* );
bool                   stream_is_reliable( stream_t* );
bool                   stream_is_inorder( stream_t* );
bool                   stream_is_swapped( stream_t* );
int                    stream_byteorder( stream_t* );
uint64_t               stream_last_modified( stream_t* );
uint64_t               stream_read( stream_t*, void*, uint64_t );
char*                  stream_read_line( stream_t*, char );
uint64_t               stream_size( stream_t* );
void                   stream_determine_binary_mode( stream_t*, unsigned int );
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
void                   stream_buffer_read( stream_t* );
unsigned int           stream_available_read( stream_t* );
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
void                   stream_truncate( stream_t*, uint64_t );
void                   stream_flush( stream_t* );
stream_t*              stream_open_stdout( void );
stream_t*              stream_open_stderr( void );
stream_t*              stream_open_stdin( void );

const char*            system_error_message( int code );
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
void                   system_process_events( void );
bool                   system_message_box( const char* title, const char* message, bool cancel_button );
event_stream_t*        system_event_stream( void );
void                   system_post_event( int event );

unsigned int           base64_encode( const void* src, char* dst, unsigned int srcsize, unsigned int dstsize );
unsigned int           base64_decode( const char* src, void* dst, unsigned int srcsize, unsigned int dstsize );

blowfish_t*            blowfish_allocate( void );
void                   blowfish_deallocate( blowfish_t* blowfish );
void                   blowfish_initialize( blowfish_t* blowfish, const void* key, const unsigned int len );
void                   blowfish_encrypt( const blowfish_t* blowfish, void* data, unsigned int length, const int mode, const uint64_t vec );
void                   blowfish_decrypt( const blowfish_t* blowfish, void* data, unsigned int length, const int mode, const uint64_t vec );

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

void                   event_post( event_stream_t* stream, uint8_t system, uint8_t id, uint16_t size, object_t object, const void* payload, tick_t delivery );
event_t*               event_next( const event_block_t* block, event_t* event );
uint16_t               event_payload_size( const event_t* event );
event_stream_t*        event_stream_allocate( unsigned int size );
void                   event_stream_deallocate( event_stream_t* stream );
event_block_t*         event_stream_process( event_stream_t* stream );

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

object_t               library_load( const char* name );
void                   library_unload( object_t library );
void*                  library_symbol( object_t library, const char* name );
const char*            library_name( object_t library );
bool                   library_valid( object_t library );

md5_t*                 md5_allocate( void );
void                   md5_deallocate( md5_t* digest );
void                   md5_initialize( md5_t* digest );
md5_t*                 md5_digest( md5_t* digest, const char* message );
md5_t*                 md5_digest_raw( md5_t* digest, const void* buffer, size_t size );
void                   md5_digest_finalize( md5_t* digest );
char*                  md5_get_digest( const md5_t* digest );
uint128_t              md5_get_digest_raw( const md5_t* digest );

stream_t*              pipe_allocate( void );
void                   pipe_close_read( stream_t* pipe );
void                   pipe_close_write( stream_t* pipe );

process_t*             process_allocate( void );
void                   process_deallocate( process_t* proc );
void                   process_set_working_directory( process_t* proc, const char* path );
void                   process_set_executable_path( process_t* proc, const char* path );
void                   process_set_arguments( process_t* proc, const char** args, unsigned int num );
void                   process_set_flags( process_t* proc, unsigned int flags );
int                    process_spawn( process_t* proc );
stream_t*              process_stdout( process_t* proc );
stream_t*              process_stdin( process_t* proc );
int                    process_wait( process_t* proc );
void                   process_set_verb( process_t* proc, const char* verb );
int                    process_exit_code( void );
void                   process_set_exit_code( int code );
void                   process_exit( int code );

uint32_t               random32( void );
uint32_t               random32_range( uint32_t low, uint32_t high );
uint64_t               random64( void );
uint64_t               random64_range( uint64_t low, uint64_t high );
real                   random_normalized( void );
real                   random_range( real low, real high );
int32_t                random32_gaussian_range( int32_t low, int32_t high );
real                   random_gaussian_range( real low, real high );
int32_t                random32_triangle_range( int32_t low, int32_t high );
real                   random_triangle_range( real low, real high );
uint32_t               random32_weighted( uint32_t limit, const real* weights );

ringbuffer_t*          ringbuffer_allocate( unsigned int size );
void                   ringbuffer_deallocate( ringbuffer_t* buffer );
unsigned int           ringbuffer_size( ringbuffer_t* size );
void                   ringbuffer_reset( ringbuffer_t* buffer );
unsigned int           ringbuffer_read( ringbuffer_t* buffer, void* dest, unsigned int num );
unsigned int           ringbuffer_write( ringbuffer_t* buffer, const void* source, unsigned int num );
uint64_t               ringbuffer_total_read( ringbuffer_t* buffer );
uint64_t               ringbuffer_total_written( ringbuffer_t* buffer );
stream_t*              ringbuffer_stream_allocate( unsigned int buffer_size, uint64_t total_size );

tick_t                 time_current( void );
tick_t                 time_diff( const tick_t from, const tick_t to );
deltatime_t            time_elapsed( const tick_t since );
tick_t                 time_elapsed_ticks( const tick_t since );
tick_t                 time_ticks_per_second( void );
deltatime_t            time_ticks_to_seconds( const tick_t dt );
tick_t                 time_startup( void );
tick_t                 time_system( void );

]]

function hash( str ) return C.hash( str, C.string_length( str ) ) end

function string_array_to_table( arr )
      local tab = {}
      local arr_size = C.array_size( arr )
      for i=1,arr_size do
            tab[i] = ffi.string( C.array_element_pointer( arr, i-1 ) )
      end
      return tab
end

function string_table_to_array( tab )
      local num = 0
      while tab[num+1] ~= nil do num = num + 1 end
      local arr = C.array_allocate_pointer( num )
      for i=0,num do
      	  C.array_set_element_pointer( arr, i, tab[i+1] )
      end
      return arr, num
end

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

-- Context hash value is hash of "lua", since luajit parser does not handle
-- full 64bit hex numbers (numbers > 0xc000000000000000 are truncated)
HASH_LUA = hash( "lua" )

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

--process.set_arguments = void                   process_set_arguments( process_t* proc, const char** args, unsigned int num );
