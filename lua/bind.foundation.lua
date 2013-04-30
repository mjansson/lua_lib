
local ffi = require( "ffi" )

ffi.cdef[[

void log_debugf( const char*, ... );
void log_infof( const char*, ... );
void log_warnf( int, const char*, ... );
void log_errorf( int, int, const char*, ... );

]]

local function log_debug( message ) ffi.C.log_debugf( "%s", message ) end
local function log_info( message ) ffi.C.log_infof( "%s", message ) end
local function log_warn( message ) ffi.C.log_warnf( 6, "%s", message ) end -- 6 = WARNING_SCRIPT
local function log_error( message ) ffi.C.log_errorf( 4, 11, "%s", message ) end -- 4 = ERRORLEVEL_ERROR, 11 = ERROR_SCRIPT

module("foundation")

log = {}

log.debug = log_debug
log.info = log_info
log.warn = log_warn
log.error = log_error
