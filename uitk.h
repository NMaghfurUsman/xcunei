// build windows
// get events
// inspect events
// emit events
// draw stuff

// incomplete

#include <stddef.h>
#include <stdint.h>

typedef struct TKWindow TKWindow;
typedef struct TKWindowSystem TKWindowSystem;
typedef struct TKContext TKContext;

// complete

typedef enum tk_event_type {
  TK_UNKNOWN_EV ,
  TK_MESSAGE_EV ,
  TK_FATAL_EV ,

  TK_WINDOW_EV ,

  TK_MOUSE_MOTION_EV,
  TK_MOUSE_ENTER_EV,
  TK_MOUSE_LEAVE_EV,

  TK_KEYBOARD_EV
}
  TKEventType;

typedef enum tk_mouse_button {
  TK_MOUSE_1 = 1,
  TK_MOUSE_2 = 2,
  TK_MOUSE_3 = 4,
} TKButtoonMask;

typedef enum tk_button_state {
  TK_BUTTON_DOWN = 0,
  TK_BUTTON_UP = 1,
} TKButtonState;

typedef enum tk_key_state {
  TK_KEY_DOWN = 0,
  TK_KEY_UP = 1,
} TKKeyState;

typedef struct {
  int x;
  int y;
} TKCoord;

typedef struct {
  TKEventType event_type;

  int keycode;
  TKKeyState k_state;

  uint8_t buttons;
  int b_state;
  TKCoord coord;

} TKEvent;

typedef struct {
  int x;
  int y;
  size_t w;
  size_t h;
} TKRect;

typedef struct {
  short r;
  short g;
  short b;
} TKColor;

TKWindowSystem* tk_init_windowing();
void tk_deinit_windowing(const TKWindowSystem* ws);

TKWindow* tk_create_window(const TKWindowSystem* ws,  size_t width, size_t height, const char* name);
void tk_destroy_window(const TKWindowSystem* ws,  TKWindow* w);
TKWindow* tk_focused_window(const TKWindowSystem* ws);
void tk_show_window(const TKWindowSystem* ws, TKWindow* w);

TKEvent* tk_wait_for_next_event(const TKWindowSystem* ws);
void tk_emit_key_event(const TKWindowSystem* ws, char keycode);

TKContext* tk_create_context(const TKWindowSystem* ws, TKWindow* drawable, TKColor fill_col, TKColor line_col, size_t width);
void tk_draw_rect(const TKWindowSystem* ws, TKWindow* w, TKContext* gc, TKRect rect);
void tk_draw_rects(const TKWindowSystem* ws, TKWindow* w, TKContext* gc, unsigned int num_rects, TKRect* rect);
void tk_draw_segment(const TKWindowSystem* ws, TKWindow* w, TKContext* gc, TKCoord a, TKCoord b);
void tk_draw_segments(const TKWindowSystem* ws, TKWindow* w, TKContext* gc, unsigned int num_coords, TKCoord* coords);

void tk_draw_text(const TKWindowSystem* ws, TKWindow *w, const char* text, unsigned int x, unsigned int y);
void tk_draw_small_text(const TKWindowSystem* ws, TKWindow *w, const char* text, unsigned int x, unsigned int y);

TKRect tk_window_region(const TKWindowSystem* ws, TKWindow* w);
void tk_clear_window(const TKWindowSystem* ws, TKWindow* w);
