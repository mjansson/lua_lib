/* errorcodes.h  -  Lua luadump tool  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
 * 
 * This library provides a cross-platform foundation library in C11 providing basic support data types and
 * functions to write applications and games in a platform-independent fashion. The latest source code is
 * always available at
 * 
 * https://github.com/rampantpixels/foundation_lib
 * 
 * This library is put in the public domain; you can redistribute it and/or modify it without any restrictions.
 *
 */


//Error codes returned by luadump tool
#define LUADUMP_RESULT_OK                              0
#define LUADUMP_RESULT_UNABLE_TO_OPEN_INPUT_FILE      -1
#define LUADUMP_RESULT_UNABLE_TO_OPEN_OUTPUT_FILE     -2
#define LUADUMP_RESULT_UNABLE_TO_LOAD_JITLIB_VMDEF    -3
#define LUADUMP_RESULT_UNABLE_TO_LOAD_JITLIB_BC       -4
#define LUADUMP_RESULT_FAILED_EVAL                    -5
