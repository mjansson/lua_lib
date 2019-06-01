
local ffi = require("ffi")
local C = ffi.C

local foundation = require("foundation")

-- Window types and functions
ffi.cdef[[

typedef struct window_config_t window_config_t;
typedef struct window_t window_t;

typedef void (* window_draw_fn)(window_t*);

int window_module_initialize(const window_config_t);
void window_module_finalize(void);
bool window_module_is_initialized(void);
version_t window_module_version(void);

window_t* window_create(unsigned int, const char*, size_t, int, int, unsigned int);
window_t* window_allocate(void*);
void window_initialize(window_t*, void*);
void window_finalize(window_t*);
void window_deallocate(window_t*);
unsigned int window_adapter(window_t*);
void window_maximize(window_t*);
void window_minimize(window_t*);
void window_restore(window_t*);
void window_resize(window_t*, int, int);
void window_move(window_t*, int, int);
bool window_is_open(window_t*);
bool window_is_visible(window_t*);
bool window_is_maximized(window_t*);
bool window_is_minimized(window_t*);
bool window_has_focus(window_t*);
void window_show_cursor(window_t*, bool, bool);
void window_set_cursor_pos(window_t*, int, int);
bool window_is_cursor_locked(window_t*);
void window_set_title(window_t*, const char*, size_t);
int window_width(window_t*);
int window_height(window_t*);
int window_position_x(window_t*);
int window_position_y(window_t*);
void window_fit_to_screen(window_t*);

void* window_hwnd(window_t*);
void* window_hinstance(window_t*);
void* window_hdc(window_t*);
void window_release_hdc(void*, void*);

void* window_display(window_t*);
void* window_content_view(window_t*);
unsigned long window_drawable(window_t*);
void* window_visual(window_t*);
int window_screen(window_t*);
void* window_view(window_t*, unsigned int);
void* window_layer(window_t*, void*);

void window_add_displaylink(window_t*, window_draw_fn);

void window_show_keyboard(window_t*);
void window_hide_keyboard(window_t*);

int window_screen_width(unsigned int);
int window_screen_height(unsigned int);

int window_view_width(window_t*, void*);
int window_view_height(window_t*, void*);

]]

return {

-- Constants
WINDOWEVENT_CREATE = 1,
WINDOWEVENT_RESIZE = 2,
WINDOWEVENT_CLOSE = 3,
WINDOWEVENT_DESTROY = 4,
WINDOWEVENT_SHOW = 5,
WINDOWEVENT_HIDE = 6,
WINDOWEVENT_GOTFOCUS = 7,
WINDOWEVENT_LOSTFOCUS = 8,
WINDOWEVENT_REDRAW = 9,

WINDOW_ADAPTER_DEFAULT = -1

}
