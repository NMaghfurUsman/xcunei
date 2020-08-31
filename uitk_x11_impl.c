#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h> // can't load fonts without it
#include <xcb/xcb_icccm.h> // easy way to set WM_HINTS
#include "uitk.h"
#include <stdlib.h> // malloc
#include <stdio.h> // printf
#include <string.h> // strlen
#include <inttypes.h> // uint32_t
#include <xcb/xtest.h> // send fake input

static int tk_get_atom_id(xcb_connection_t* c, const char* name) {
  xcb_intern_atom_cookie_t cookie;
  xcb_intern_atom_reply_t* reply;

  cookie = xcb_intern_atom(c, 0, strlen(name), name);

  while (reply == NULL) {
    reply = xcb_intern_atom_reply(c, cookie, NULL);
  }

  return reply->atom;
}

static void tk_set_wm_hint(xcb_connection_t* c, xcb_window_t w) {
  xcb_icccm_wm_hints_t hints;
  hints.flags = hints.flags ^ XCB_ICCCM_WM_HINT_INPUT;
  xcb_icccm_wm_hints_set_input(&hints, 0x00);

  xcb_icccm_set_wm_hints(c, w, &hints);
}

struct TKWindowSystem {
  xcb_connection_t* conn;
  xcb_screen_t* screen;
  Display* dpy;
};

struct TKWindow {
  xcb_window_t id;
};

struct TKContext {
  xcb_gcontext_t id;
};

TKWindowSystem* tk_init_windowing() {
  TKWindowSystem* ws = malloc(sizeof(TKWindowSystem));
  
  Display *dpy = XOpenDisplay(NULL);

  ws->conn = XGetXCBConnection(dpy);
  ws->dpy = dpy;
  ws->screen = xcb_setup_roots_iterator (xcb_get_setup (ws->conn)).data;

  return ws;
}

void tk_deinit_windowing(const TKWindowSystem* ws) {
  xcb_disconnect(ws->conn);
}

TKWindow* tk_create_window(const TKWindowSystem* ws, size_t width, size_t height, const char* name) {
  TKWindow* w = malloc(sizeof(xcb_window_t));

  w->id = xcb_generate_id(ws->conn);

  uint32_t     mask      = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t     values[2] = {ws->screen->white_pixel,
    XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
    XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
    XCB_EVENT_MASK_ENTER_WINDOW   | XCB_EVENT_MASK_LEAVE_WINDOW
    | XCB_EVENT_MASK_STRUCTURE_NOTIFY 
    /* XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE */ };

  xcb_create_window (ws->conn,    
                     0,                             /* depth               */
                     w->id,                        
                     ws->screen->root,              /* parent window       */
                     0, 0,                          /* x, y                */
                     width, height,                 /* width, height       */
                     10,                            /* border_width        */
                     XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                     ws->screen->root_visual,       /* visual              */
                     mask, values );                /* masks */

  xcb_change_property(ws->conn, XCB_PROP_MODE_REPLACE, w->id, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(name), name);

  tk_set_wm_hint(ws->conn, w->id);
  
  return w;
}

void tk_destroy_window(const TKWindowSystem* ws, TKWindow* w) {
  xcb_destroy_window(ws->conn, w->id);
}

void tk_show_window(const TKWindowSystem* ws, TKWindow* w){
  xcb_map_window(ws->conn, w->id);
  xcb_flush(ws->conn);
}

TKEvent* tk_wait_for_next_event(const TKWindowSystem* ws) {

  xcb_generic_event_t* event;

  TKEvent* ev = calloc(sizeof(TKEvent),1);

  event = xcb_wait_for_event(ws->conn);

  if (event == NULL) {
    ev->event_type = TK_FATAL_EV;
    return ev;
  }

  switch (event->response_type & ~0x80) {
  case XCB_CLIENT_MESSAGE: {
    ev->event_type = TK_MESSAGE_EV;
    break;
  }
  case XCB_CONFIGURE_NOTIFY: {
    ev->event_type = TK_WINDOW_EV;
    xcb_configure_notify_event_t* conf;
    conf = (xcb_configure_notify_event_t*)event;
    break;
  }
  case XCB_MAP_NOTIFY: {
    ev->event_type = TK_WINDOW_EV;
    xcb_map_notify_event_t* map;
    map = (xcb_map_notify_event_t*)event;
    break;
  }
  case XCB_EXPOSE: {
    ev->event_type = TK_WINDOW_EV;
    xcb_expose_event_t* expose;
    expose = (xcb_expose_event_t*)event;
    break;
  }

  case XCB_BUTTON_PRESS: {
    xcb_button_press_event_t* press;
    press = (xcb_button_press_event_t*)event;
    TKCoord coord =  {press->event_x, press->event_y};
    ev->coord = coord;
    ev->event_type = TK_MOUSE_MOTION_EV;
    ev->buttons = press->state >> 8;
    ev->b_state = 1;
    break;
  }
  case XCB_BUTTON_RELEASE: {
    xcb_button_press_event_t* press;
    press = (xcb_button_press_event_t*)event;
    TKCoord coord =  {press->event_x, press->event_y};
    ev->coord = coord;
    ev->event_type = TK_MOUSE_MOTION_EV;
    ev->buttons = press->state >> 8;
    ev->b_state = -1;
    break;
  }
  case XCB_MOTION_NOTIFY: {
    xcb_motion_notify_event_t* motion;
    motion = (xcb_motion_notify_event_t*)event;
    TKCoord coord =  {motion->event_x, motion->event_y};
    ev->coord = coord;
    ev->event_type = TK_MOUSE_MOTION_EV;
    ev->buttons = motion->state >> 8;
    ev->b_state = 0;
    break;
  }
  case XCB_ENTER_NOTIFY: {
    ev->event_type = TK_MOUSE_ENTER_EV;
    break;
  }
  case XCB_LEAVE_NOTIFY: {
    ev->event_type = TK_MOUSE_LEAVE_EV;
    break;
  }
  case XCB_KEY_PRESS:
  case XCB_KEY_RELEASE: {
    xcb_key_press_event_t* keyp;
    keyp = (xcb_key_press_event_t*)event;
    ev->event_type = TK_KEYBOARD_EV;
    ev->keycode = keyp->detail;
    break;
  }
  default:
    ev->event_type = 0;
  }

  free(event);

  return ev;
}

TKContext* tk_create_context(const TKWindowSystem* ws, TKWindow* drawable, TKColor fill_col, TKColor line_col, size_t width) {
  xcb_gcontext_t gc = xcb_generate_id(ws->conn);

  uint32_t col = 0x000000;

  col = col | fill_col.r << 16;
  col = col | fill_col.g << 8;
  col = col | fill_col.b;

  /* Create a graphic context for drawing in the foreground */
  xcb_screen_t* screen = xcb_setup_roots_iterator (xcb_get_setup (ws->conn)).data;
  xcb_drawable_t win = screen->root;
  uint32_t mask = XCB_GC_BACKGROUND | XCB_GC_FOREGROUND | XCB_GC_LINE_WIDTH ;
  uint32_t value[3];
  value[0] = col;
  value[1] = screen->black_pixel; // idk why i need to do this, but its necessary for color to work
  value[2] = width;
  xcb_create_gc (ws->conn, gc, win, mask, value);

  TKContext* tkgc = malloc(sizeof(TKContext));
  tkgc->id = gc;
  return tkgc;
}

void tk_draw_rect(const TKWindowSystem* ws, TKWindow* w, TKContext* gc, TKRect rect) {
  xcb_rectangle_t rects[1] = {{rect.x, rect.y, rect.w, rect.h}};

  xcb_poly_rectangle(ws->conn, w->id, gc->id, 1, rects);
  xcb_flush(ws->conn);
}

void tk_draw_rects(const TKWindowSystem* ws, TKWindow* w, TKContext* gc, unsigned int num_rects, TKRect* rect) {
  xcb_rectangle_t xcb_rects[num_rects];

  for (int i = 0; i<num_rects; i++) {
    xcb_rectangle_t r = {rect[i].x, rect[i].y, rect[i].w, rect[i].h};
    xcb_rects[i] = r;
  }

  xcb_poly_rectangle(ws->conn, w->id, gc->id, num_rects, xcb_rects);
  xcb_flush(ws->conn);
}

void tk_draw_segment(const TKWindowSystem* ws, TKWindow* w, TKContext* gc, TKCoord a, TKCoord b) {
  xcb_segment_t segments[1] = {{a.x, a.y, b.x, b.y}};

  xcb_poly_segment(ws->conn, w->id, gc->id, 1, segments);
  xcb_flush(ws->conn);
}

void tk_draw_segments(const TKWindowSystem* ws, TKWindow* w, TKContext* gc, unsigned int num_coords, TKCoord* coords) {
  xcb_point_t xcb_coords[num_coords];

  for (int i = 0; i<num_coords; i++) {
    xcb_point_t p = {coords[i].x, coords[i].y};
    xcb_coords[i] = p;
  }

  xcb_poly_line(ws->conn,XCB_COORD_MODE_ORIGIN, w->id, gc->id, num_coords, xcb_coords);
  xcb_flush(ws->conn);
}

TKWindow* tk_focused_window(const TKWindowSystem* ws) {
  xcb_get_input_focus_cookie_t cookie;
  xcb_get_input_focus_reply_t* reply;
  xcb_generic_error_t* error;

  cookie = xcb_get_input_focus(ws->conn);
  TKWindow* w = malloc(sizeof(TKWindow));

  if((  reply = xcb_get_input_focus_reply(ws->conn, cookie, &error) )) {
    w->id = reply->focus;
  } else {
    printf("X11 Error %d\n", error->error_code);
  }

  return w;
}

static void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

void tk_emit_key_event(const TKWindowSystem* ws, char keycode) {
  TKWindow* focused = tk_focused_window(ws);
  int shift = ((int)keycode >= 65 && (int)keycode<= 90);

  uint8_t k = XKeysymToKeycode(ws->dpy, keycode);

  if (keycode == '\b') {
    k = 22;
  }

  if (shift) {
    xcb_test_fake_input(ws->conn, XCB_KEY_PRESS, 50, XCB_CURRENT_TIME, focused->id, 0,0,0);
    xcb_flush(ws->conn);
  }

  xcb_test_fake_input(ws->conn, XCB_KEY_PRESS, k, XCB_CURRENT_TIME, focused->id, 0,0,0);
  xcb_test_fake_input(ws->conn, XCB_KEY_RELEASE, k, XCB_CURRENT_TIME, focused->id, 0,0,0);

  if (shift) {
    xcb_test_fake_input(ws->conn, XCB_KEY_RELEASE, 50, XCB_CURRENT_TIME, focused->id, 0,0,0);
  }
  xcb_flush(ws->conn);
}

TKRect tk_window_region(const TKWindowSystem* ws, TKWindow* w) {
  xcb_get_geometry_cookie_t cookie;
  xcb_get_geometry_reply_t* reply;
  xcb_generic_error_t* error;

  cookie = xcb_get_geometry(ws->conn, w->id);
  xcb_flush(ws->conn);

  TKRect rect;
  if((  reply = xcb_get_geometry_reply(ws->conn, cookie, &error) )) {
    rect.x=reply->x;
    rect.y=reply->y;
    rect.w=reply->width;
    rect.h=reply->height;
  } else {
    printf("X11 Error %d\n", error->error_code);
  }

  return rect;
}

void tk_clear_window(const TKWindowSystem* ws, TKWindow* w) {
  TKRect rect = tk_window_region(ws, w);

  xcb_clear_area(ws->conn, 0, w->id, 0, 0, rect.w, rect.h);
  xcb_flush(ws->conn);
}

void tk_draw_text(const TKWindowSystem* ws, TKWindow *w, const char* text, unsigned int x, unsigned int y) {
  static xcb_gcontext_t gc = 0;

  if (gc == 0) {
    gc = xcb_generate_id(ws->conn);
    xcb_font_t f = xcb_generate_id(ws->conn);

    xcb_open_font(ws->conn, f, strlen("terminus-24"), "terminus-24");

    xcb_flush (ws->conn);

    /* Create a graphic context for drawing in the foreground */
    xcb_screen_t* screen = xcb_setup_roots_iterator (xcb_get_setup (ws->conn)).data;
    xcb_drawable_t win = screen->root;
    uint32_t mask = XCB_GC_BACKGROUND | XCB_GC_FOREGROUND | XCB_GC_FONT ;
    uint32_t value[3];
    value[0] = screen->black_pixel;
    value[1] = screen->white_pixel; // idk why i need to do this, but its necessary for color to work
    value[2] = f;
    xcb_create_gc (ws->conn, gc, win, mask, value);

    xcb_close_font(ws->conn, f);
    xcb_flush(ws->conn);
  }

  xcb_image_text_8(ws->conn, strlen(text), w->id, gc, x, y, text);

  xcb_flush(ws->conn);
}

void tk_draw_small_text(const TKWindowSystem* ws, TKWindow *w, const char* text, unsigned int x, unsigned int y) {
  static xcb_gcontext_t gc = 0;

  if (gc == 0) {
    gc = xcb_generate_id(ws->conn);
    xcb_font_t f = xcb_generate_id(ws->conn);

    xcb_open_font(ws->conn, f, strlen("terminus-16"), "terminus-16");

    xcb_flush (ws->conn);

    /* Create a graphic context for drawing in the foreground */
    xcb_screen_t* screen = xcb_setup_roots_iterator (xcb_get_setup (ws->conn)).data;
    xcb_drawable_t win = screen->root;
    uint32_t mask = XCB_GC_BACKGROUND | XCB_GC_FOREGROUND | XCB_GC_FONT ;
    uint32_t value[3];
    value[0] = screen->black_pixel;
    value[1] = screen->white_pixel; // idk why i need to do this, but its necessary for color to work
    value[2] = f;
    xcb_create_gc (ws->conn, gc, win, mask, value);

    xcb_close_font(ws->conn, f);
    xcb_flush(ws->conn);
  }

  xcb_image_text_8(ws->conn, strlen(text), w->id, gc, x, y, text);

  xcb_flush(ws->conn);
}
