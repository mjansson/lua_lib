
local ffi = require( "ffi" )

ffi.cdef[[

typedef struct _foundation_stream         stream_t;
typedef struct _foundation_event          event_t;
typedef struct _foundation_event_block    event_block_t;
typedef struct _foundation_event_stream   event_stream_t;
typedef struct _foundation_blowfish       blowfish_t;
typedef struct _foundation_md5            md5_t;
typedef struct _foundation_process        process_t;
typedef struct _foundation_ringbuffer     ringbuffer_t;

typedef float          float32_t;
typedef double         float64_t;
typedef float32_t      real; // Change this if compiling foundation with 64bit reals

typedef uint64_t       object_t;
typedef uint64_t       hash_t;
typedef uint64_t       tick_t;
typedef real           deltatime_t;

typedef struct { uint64_t word[2]; } uint128_t;
typedef struct { uint64_t word[4]; } uint256_t;

typedef uint128_t       uuid_t;

hash_t                 hash( const char*, unsigned int );

void                   log_debugf( uint64_t, const char*, ... );
void                   log_infof( uint64_t, const char*, ... );
void                   log_warnf( uint64_t, int, const char*, ... );
void                   log_errorf( uint64_t, int, const char*, ... );
void                   log_panicf( uint64_t, int, const char*, ... );
void                   log_enable_stdout( bool );
void                   log_enable_prefix( bool );
void                   log_set_suppress( uint64_t, int );
int                    log_suppress( uint64_t );

int                    error( void );
int                    error_report( int, int );
void                   error_context_push( const char*, const char* );
void                   error_context_pop( void );

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
void                   md5_finalize( md5_t* digest );
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

local function hashwrap( str ) return ffi.C.hash( str, ffi.C.string_length( str ) ) end

-- Context hash value is hash of "lua", since luajit parser does not handle
-- full 64bit hex numbers (numbers > 0xc000000000000000 are truncated)
local _HASH_LUA = hashwrap( "lua" ) 

local function string_array_to_table( arr )
      local tab = {}
      local arr_size = ffi.C.array_size( arr )
      for i=1,arr_size do
            tab[i] = ffi.string( ffi.C.array_element_pointer( arr, i-1 ) )
      end
      return tab
end

local function string_table_to_array( tab )
      local num = 0
      while tab[num+1] ~= nil do num = num + 1 end
      local arr = ffi.C.array_allocate_pointer( num )
      for i=0,num do
      	  ffi.C.array_set_element_pointer( arr, i, tab[i+1] )
      end
      return arr, num
end


module("foundation")

PLATFORM_WINDOWS = 1
PLATFORM_LINUX = 2
PLATFORM_MACOSX = 3
PLATFORM_IOS = 4
PLATFORM_ANDROID = 5
PLATFORM_RASPBERRYPI = 6

ARCHITECTURE_X86 = 0
ARCHITECTURE_X86_64 = 0
ARCHITECTURE_PPC = 0
ARCHITECTURE_PPC_64 = 0
ARCHITECTURE_ARM6 = 0
ARCHITECTURE_ARM7 = 0

BYTEORDER_LITTLEENDIAN = 0
BYTEORDER_BIGENDIAN = 1

FOUNDATIONEVENT_TERMINATE = 1
FOUNDATIONEVENT_FILE_CREATED = 2
FOUNDATIONEVENT_FILE_DELETED = 3
FOUNDATIONEVENT_FILE_MODIFIED = 4

hash = hashwrap

log = {}
log.debug = function ( message ) ffi.C.log_debugf( _HASH_LUA, "%s", message ) end
log.info = function ( message ) ffi.C.log_infof( _HASH_LUA, "%s", message ) end
log.warn = function ( message ) ffi.C.log_warnf( _HASH_LUA, 8, "%s", message ) end -- 8 = WARNING_SCRIPT
log.error = function ( message ) ffi.C.log_errorf( _HASH_LUA, 15, "%s", message ) end -- 15 = ERROR_SCRIPT
log.panic = function ( message ) ffi.C.log_panicf( _HASH_LUA, 15, "%s", message ) end -- 15 = ERROR_SCRIPT
log.enable_stdout = ffi.C.log_enable_stdout
log.enable_prefix = ffi.C.log_enable_prefix
log.set_suppress = function ( context, level ) ffi.C.log_set_suppress( hashwrap( context ), level ) end
log.suppress = function ( context ) return ffi.C.log_suppress( hashwrap( context ) ) end

error = {}
error.get = ffi.C.error
error.report = ffi.C.error_report
error.context_push = ffi.C.error_context_push
error.context_pop = ffi.C.error_context_pop
error.LEVEL_NONE = 0
error.LEVEL_DEBUG = 1
error.LEVEL_INFO = 2
error.LEVEL_WARNING = 3
error.LEVEL_ERROR = 4
error.LEVEL_PANIC = 5
error.NONE = 0
error.INVALID_VALUE = 1
error.UNSUPPORTED = 2
error.NOT_IMPLEMENTED = 3
error.OUT_OF_MEMORY = 4
error.MEMORY_LEAK = 5
error.MEMORY_ALIGNMENT = 6
error.INTERNAL_FAILURE = 7
error.ACCESS_DENIED = 8
error.EXCEPTION = 9
error.SYSTEM_CALL_FAIL = 10
error.UNKNOWN_TYPE = 11
error.UNKNOWN_RESOURCE = 12
error.DEPRECATED = 13
error.ASSERT = 14
error.SCRIPT = 15
error.WARNING_PERFORMANCE = 0
error.WARNING_DEPRECATED = 1
error.WARNING_BAD_DATA = 2
error.WARNING_MEMORY = 3
error.WARNING_UNSUPPORTED = 4
error.WARNING_SUSPICIOUS = 5
error.WARNING_SYSTEM_CALL_FAIL = 6
error.WARNING_DEADLOCK = 7
error.WARNING_SCRIPT = 8

string = {}
string.deallocate = ffi.C.string_deallocate
string.array_to_table = string_array_to_table
string.table_to_array = string_table_to_array
string.explode = function ( str, delimiters, allow_empty ) local arr = ffi.C.string_explode( str, delimiters, allow_empty ); local tab = string_array_to_table( arr ); ffi.C.array_deallocate( arr ); return tab end
string.merge = function ( tab, delimiter ) local arr, num = string_table_to_array( tab ); local cstr = ffi.C.string_merge( arr, num, delimiter ); local str = ffi.string( cstr ); ffi.C.string_deallocate( cstr ); ffi.C.array_deallocate( arr ); return str end
string.from_uint128 = function ( val ) ffi.string( ffi.C.string_from_uint128_static( val ) ) end
string.from_time = function ( time ) ffi.string( ffi.C.string_from_time_static( time ) ) end
string.from_uuid = function ( uuid ) ffi.string( ffi.C.string_from_uuid_static( uuid ) ) end
string.to_uint64 = ffi.C.string_to_uint64
string.to_uint128 = ffi.C.string_to_uint128
string.to_uuid = ffi.C.string_to_uuid
string.is_valid_email_address = ffi.C.string_is_valid_email_address

environment = {}
environment.command_line = function () local arr = ffi.C.environment_command_line(); return string_array_to_table( arr ) end
environment.executable_name = function () return ffi.string( ffi.C.environment_executable_name() ) end
environment.executable_directory = function () return ffi.string( ffi.C.environment_executable_directory() ) end 
environment.initial_working_directory = function () return ffi.string( ffi.C.environment_initial_working_directory() ) end
environment.current_working_directory = function () return ffi.string( ffi.C.environment_current_working_directory() ) end
environment.set_current_working_directory = function () return ffi.string( ffi.C.environment_set_current_working_directory() ) end
environment.home_directory = function () return ffi.string( ffi.C.environment_home_directory() ) end
environment.temporary_directory = function () return ffi.string( ffi.C.environment_temporary_directory() ) end
environment.variable = function ( var ) return ffi.string( ffi.C.environment_variable( var ) ) end

stream = {}
stream.IN = 1
stream.OUT = 2
stream.INOUT = 3
stream.TRUNCATE = 4
stream.ATEND = 8
stream.BINARY = 16
stream.SYNC = 32
stream.SEEK_BEGIN = 0
stream.SEEK_CURRENT = 1
stream.SEEK_END = 2
stream.open = ffi.C.stream_open
stream.clone = ffi.C.stream_clone
stream.deallocate = ffi.C.stream_deallocate
stream.tell = ffi.C.stream_tell
stream.seek = ffi.C.stream_seek
stream.eos = ffi.C.stream_eos
stream.byteorder = ffi.C.stream_set_byteorder
stream.binary = ffi.C.stream_set_binary
stream.is_sequential = ffi.C.stream_is_sequential
stream.is_reliable = ffi.C.stream_is_reliable
stream.is_inorder = ffi.C.stream_is_inorder
stream.is_swapped = ffi.C.stream_is_swapped
stream.byteorder = ffi.C.stream_byteorder
stream.last_modified = ffi.C.stream_last_modified
stream.read = ffi.C.stream_read
stream.read_line = function ( stream, delimiter ) local line = ffi.C.stream_read_line( stream, delimiter ); local str = ffi.string( line ); string.deallocate( line ); return str end
stream.size = ffi.C.stream_size
stream.determine_binary_mode = ffi.C.stream_determine_binary_mode
stream.read_bool = ffi.C.stream_read_bool
stream.read_int8 = ffi.C.stream_read_int8
stream.read_uint8 = ffi.C.stream_read_uint8
stream.read_int16 = ffi.C.stream_read_int16
stream.read_uint16 = ffi.C.stream_read_uint16
stream.read_int32 = ffi.C.stream_read_int32
stream.read_uint32 = ffi.C.stream_read_uint32
stream.read_int64 = ffi.C.stream_read_int64
stream.read_uint64 = ffi.C.stream_read_uint64
stream.read_float32 = ffi.C.stream_read_float32
stream.read_float64 = ffi.C.stream_read_float64
stream.read_string = function ( stream ) local line = ffi.C.stream_read_string( stream ); local str = ffi.string( line ); string.deallocate( line ); return str end
stream.buffer_read = ffi.C.stream_buffer_read
stream.available_read = ffi.C.stream_available_read
stream.write_bool = ffi.C.stream_write_bool
stream.write_int8 = ffi.C.stream_write_int8
stream.write_uint8 = ffi.C.stream_write_uint8
stream.write_int16 = ffi.C.stream_write_int16
stream.write_uint16 = ffi.C.stream_write_uint16
stream.write_int32 = ffi.C.stream_write_int32
stream.write_uint32 = ffi.C.stream_write_uint32
stream.write_in64 = ffi.C.stream_write_int64
stream.write_uint64 = ffi.C.stream_write_uint64
stream.write_float32 = ffi.C.stream_write_float32
stream.write_float64 = ffi.C.stream_write_float64
stream.write_string = ffi.C.stream_write_string
stream.write_endl = ffi.C.stream_write_endl
stream.truncate = ffi.C.stream_truncate
stream.flush = ffi.C.stream_flush
stream.open_stdout = ffi.C.stream_open_stdout
stream.open_stderr = ffi.C.stream_open_stderr
stream.open_stdin = ffi.C.stream_open_stdin

system = {}
system.error_message = function () return ffi.string( ffi.C.system_error_message() ) end
system.platform = ffi.C.system_platform
system.architecture = ffi.C.system_architecture
system.byteorder = ffi.C.system_byteorder
system.hardware_threads = ffi.C.system_hardware_threads
system.hostname = function () return ffi.string( ffi.C.system_hostname() ) end
system.hostid = ffi.C.system_hostid
system.username = function () return ffi.string( ffi.C.system_username() ) end
system.debugger_attached = ffi.C.system_debugger_attached
system.pause = ffi.C.system_pause
system.language = ffi.C.system_language
system.country = ffi.C.system_country
system.locale = ffi.C.system_locale
system.locale_string = function () return ffi.string( ffi.C.system_locale_string() ) end
system.process_events = ffi.C.system_process_events
system.message_box = ffi.C.system_message_box
system.event_stream = ffi.C.system_event_stream
system.post_event = ffi.C.system_post_event

base64 = {}
base64.encode = ffi.C.base64_encode
base64.decode = ffi.C.base64_decode

blowfish = {}
blowfish.ECB = 0
blowfish.CBC = 1
blowfish.CFB = 2
blowfish.OFB = 3
blowfish.allocaate = ffi.C.blowfish_allocate
blowfish.deallocate = ffi.C.blowfish_deallocate
blowfish.initialize = function ( blowfish, key ) ffi.C.blowfish_initialize( blowfish, key, ffi.C.string_length( key ) ) end
blowfish.encrypt = ffi.C.blowfish_encrypt
blowfish.decrypt = ffi.C.blowfish_decrypt

config = {}
config.bool = ffi.C.config_bool
config.int = ffi.C.config_int
config.real = ffi.C.config_real
config.string = function ( section, key ) return ffi.string( ffi.C.config_string( section, key ) ) end
config.string_hash = ffi.C.config_string_hash
config.set_bool = ffi.C.config_set_bool
config.set_int = ffi.C.config_set_int
config.set_real = ffi.C.config_set_real
config.set_string = ffi.C.config_set_string
config.load = ffi.C.config_load
config.parse = ffi.C.config_parse
config.write = ffi.C.config_write
config.parse_commandline = ffi.C.config_parse_commandline

event = {}
event.post = ffi.C.event_post
event.next = ffi.C.event_next
event.payload_size = ffi.C.event_payload_size
event.stream_allocate = ffi.C.event_stream_allocate
event.stream_deallocate = ffi.C.event_stream_deallocate
event.stream_process = event_stream_process

fs = {}
fs.open_file = ffi.C.fs_open_file
fs.copy_file = ffi.C.fs_copy_file
fs.remove_file = ffi.C.fs_remove_file
fs.temporary_file = ffi.C.fs_temporary_file
fs.is_file = ffi.C.fs_is_file
fs.make_directory = ffi.C.fs_make_directory
fs.remove_directory = ffi.C.fs_remove_directory
fs.is_directory = ffi.C.fs_is_directory
fs.last_modified = ffi.C.fs_last_modified
fs.touch = ffi.C.fs_touch
fs.md5 = ffi.C.fs_md5
fs.matching_files = function ( path, pattern, recurse ) local arr = ffi.C.fs_matching_files( path, pattern, recurse ); local table = string_array_to_table( arr ); ffi.C.array_deallocate( arr ); return table end
fs.files = function ( path ) local arr = ffi.C.fs_files( path ); local table = string_array_to_table( arr ); ffi.C.array_deallocate( arr ); return table end
fs.subdirs = function ( path ) local arr = ffi.C.fs_subdirs( path ); local table = string_array_to_table( arr ); ffi.C.array_deallocate( arr ); return table end
fs.monitor = ffi.C.fs_monitor
fs.unmonitor = ffi.C.fs_unmonitor
fs.post_event = function ( id, path ) ffi.C.fs_post_event( id, path, ffi.C.string_length( path ) ) end
fs.event_stream = ffi.C.fs_event_stream

library = {}
library.load = ffi.C.library_load
library.unload = ffi.C.library_unload
library.symbol = ffi.C.library_symbol
library.name = function ( library ) return ffi.string( ffi.C.library_name( library ) ) end
library.valid = ffi.C.library_valid

md5 = {}
md5.allocate = ffi.C.md5_allocate
md5.deallocate = ffi.C.md5_deallocate
md5.initialize = ffi.C.md5_initialize
md5.digest = ffi.C.md5_digest
md5.digest_raw = ffi.C.md5_digest_raw
md5.finalize = ffi.C.md5_finalize
md5.get_digest = function ( md5 ) return ffi.string( ffi.C.md5_get_digest( digest ) ) end
md5.get_digest_raw = ffi.C.md5_get_digest_raw

pipe = {}
pipe.allocate = ffi.C.pipe_allocate
pipe.close_read = ffi.C.pipe_close_read
pipe.close_write = ffi.C.pipe_close_write

process = {}
process.ATTACHED                          = 0
process.DETACHED                          = 0x01
process.CONSOLE                           = 0x02
process.STDSTREAMS                        = 0x04
process.WINDOWS_USE_SHELLEXECUTE          = 0x08
process.OSX_USE_OPENAPPLICATION           = 0x10
process.INVALID_ARGS                      = 0x7FFFFFF0
process.TERMINATED_SIGNAL                 = 0x7FFFFFF1
process.WAIT_INTERRUPTED                  = 0x7FFFFFF2
process.WAIT_FAILED                       = 0x7FFFFFF3
process.STILL_ACTIVE                      = 0x7FFFFFFF
process.allocate = ffi.C.process_allocate
process.deallocate = ffi.C.process_deallocate
process.set_working_directory = ffi.C.process_set_working_directory
process.set_executable_path = ffi.C.process_set_executable_path
--process.set_arguments = void                   process_set_arguments( process_t* proc, const char** args, unsigned int num );
process.set_flags = ffi.C.process_set_flags
process.spawn = ffi.C.process_spawn
process.stdout = ffi.C.process_stdout
process.stdin = ffi.C.process_stdin
process.wait = ffi.C.process_wait
process.set_verb = ffi.C.process_set_verb
process.exit_code = ffi.C.process_exit_code
process.set_exit_code = ffi.C.process_set_exit_code
process.exit = ffi.C.process_exit

random = {}
random.uint32 = ffi.C.random32
random.range32 = ffi.C.random32_range
random.uint64 = ffi.C.random64
random.range64 = ffi.C.random64_range
random.normalized = ffi.C.random_normalized
random.range = ffi.C.random_range
random.gaussian_range32 = ffi.C.random32_gaussian_range
random.gaussian_range = ffi.C.random_gaussian_range
random.triangle_range32 = ffi.C.random32_triangle_range
random.triangle_range = ffi.C.random_triangle_range
--random.weighted32 = uint32_t               random32_weighted( uint32_t limit, const real* weights )

ringbuffer = {}
ringbuffer.allocate = ffi.C.ringbuffer_allocate
ringbuffer.deallocate = ffi.C.ringbuffer_deallocate
ringbuffer.size = ffi.C.ringbuffer_size
ringbuffer.reset = ffi.C.ringbuffer_reset
ringbuffer.read = ffi.C.ringbuffer_read
ringbuffer.write = ffi.C.ringbuffer_write
ringbuffer.total_read = ffi.C.ringbuffer_total_read
ringbuffer.total_writen = ffi.C.ringbuffer_total_written
ringbuffer.stream_allocate = ffi.C.ringbuffer_stream_allocate

time = {}
time.current = ffi.C.time_current
time.diff = ffi.C.time_diff
time.elapsed = ffi.C.time_elapsed
time.elapsed_ticks = ffi.C.time_elapsed_ticks
time.ticks_per_second = ffi.C.time_ticks_per_second
time.ticks_to_seconds = ffi.C.time_ticks_to_seconds
time.startup = ffi.C.time_startup
time.system = ffi.C.time_system
