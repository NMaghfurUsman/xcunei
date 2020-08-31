#include "uitk.h"
#include <stdlib.h>
#include <stdio.h>
#include <tgmath.h>

typedef enum {
  XC_1,
  XC_2,
  XC_3,
  XC_4,
  XC_5,
  XC_6,
  XC_7,
  XC_8,
  XC_9,
  XC_SPACE,
  XC_DELETE,
  XC_NOOP
} xc_region;

inline static int max(int a, int b) {
  if (a > b) { return a; } else {return b;}
}

static xc_region parse_coord(TKCoord coord, TKRect r) {
  unsigned int near_x, mid_x, far_x;
  unsigned int near_y, mid_y, far_y;
  
  near_x = r.w / 4;
  near_y = r.h / 4;
  
  mid_x = near_x * 2;
  mid_y = near_y * 2;
  
  far_x = near_x * 3;
  far_y = near_y * 3;
  
  if (coord.y > far_y) {
    return XC_SPACE;
  } else {
    
    if (coord.x > far_x) {
      return XC_DELETE;
    } else {
      
      if (coord.x < near_x) {
        if (coord.y < near_y) {return XC_1;}
        if (coord.y > near_y && coord.y < mid_y) {return XC_4;}
        if (coord.y > mid_y && coord.y < far_y) {return XC_7;}
      }
      
      if (coord.x > near_x && coord.x < mid_x) {
        if (coord.y < near_y) {return XC_2;}
        if (coord.y > near_y && coord.y < mid_y) {return XC_5;}
        if (coord.y > mid_y && coord.y < far_y) {return XC_8;}
      }
      
      if (coord.x > mid_x && coord.x < far_x) {
        if (coord.y < near_y) {return XC_3;}
        if (coord.y > near_y && coord.y < mid_y) {return XC_6;}
        if (coord.y > mid_y && coord.y < far_y) {return XC_9;}
      }
      
    }}
  
  return XC_NOOP;
}


static void xc_draw_keyboard(TKWindowSystem* ws, TKContext* gc, TKWindow* w) {
  int near_x, far_x;
  int near_y, far_y;
  
  TKRect rect = tk_window_region(ws, w);
  
  near_x = rect.w / 4;
  far_x = near_x + near_x;
  
  near_y = rect.h / 4;
  far_y = near_y + near_y;
  
  TKRect rects[9] = {
    {0, 0, near_x, near_y},
    {near_x, 0, near_x, near_y},
    {far_x, 0, near_x, near_y},
    {0, near_y, near_x, near_y},
    {near_x, near_y, near_x, near_y},
    {far_x, near_y, near_x, near_y},
    {0, far_y, near_x, near_y},
    {near_x, far_y, near_x, near_y},
    {far_x, far_y, near_x, near_y}
  };
  
  int division = near_x / 3;
  int divisiony = near_y / 3;
  int half = near_x / 2;
  int halfy = near_y / 2;
  
  tk_draw_text(ws, w, "A", half-6, halfy+12);
  tk_draw_text(ws, w, "N", near_x + half-6, halfy+12);
  tk_draw_text(ws, w, "I", near_x*2 + half-6, halfy+12);
  tk_draw_text(ws, w, "H", half-6, near_y + halfy+12);
  tk_draw_text(ws, w, "O", near_x + half-6, near_y + halfy+12);
  tk_draw_text(ws, w, "R", near_x*2 + half-6, near_y + halfy+12);
  tk_draw_text(ws, w, "T", half-6, near_y*2 + halfy+12);
  tk_draw_text(ws, w, "E", near_x + half-6, near_y*2 + halfy+12);
  tk_draw_text(ws, w, "S", near_x*2 + half-6, near_y*2 + halfy+12);
  
  tk_draw_small_text(ws, w, "q", near_x+2             , near_y + 12);
  tk_draw_small_text(ws, w, "u", near_x + half-8  , near_y + 12);
  tk_draw_small_text(ws, w, "p", far_x -8, near_y + 12);
  tk_draw_small_text(ws, w, "c", near_x+2             , near_y + (near_y/2) + (12/2));
  tk_draw_small_text(ws, w, "b", far_x -8, near_y + (near_y/2) + (12/2));
  tk_draw_small_text(ws, w, "g", near_x+2             , near_y + divisiony*3);
  tk_draw_small_text(ws, w, "d", near_x + half-8  , near_y + divisiony*3);
  tk_draw_small_text(ws, w, "j", far_x -8, near_y + divisiony*3);
  
  tk_draw_small_text(ws, w, "v", near_x -8     , near_y);
  tk_draw_small_text(ws, w, "l", near_x + half-8    , near_y);
  tk_draw_small_text(ws, w, "x", far_x +2               , near_y);
  tk_draw_small_text(ws, w, "k", near_x -8             , near_y + (near_y/2) + (12/2));
  tk_draw_small_text(ws, w, "m", far_x +2, near_y + (near_y/2) + (12/2));
  tk_draw_small_text(ws, w, "y", near_x -8             , far_y + 12);
  tk_draw_small_text(ws, w, "w", near_x + half-8  , far_y +12);
  tk_draw_small_text(ws, w, "f", far_x +2, far_y +12);
  
  tk_draw_small_text(ws, w, "z", far_x -8, far_y + (near_y/2) + (12/2));
  
  tk_draw_rects(ws, w, gc, 9, rects);
}

typedef enum {
  XC_GESTURE_NONE,
  XC_GESTURE_START,
  XC_GESTURE_CONTINUE,
  XC_GESTURE_END
} xc_state;

typedef enum {
  XC_N_,
  XC_E_,
  XC_S_,
  XC_W_,
  XC_NE_,
  XC_SE_,
  XC_NW_,
  XC_SW_,
  XC_N,
  XC_E,
  XC_S,
  XC_W,
  XC_NE,
  XC_SE,
  XC_NW,
  XC_SW,
  XC_O, // circle
  XC_X // tap
} xc_gesture;

inline static size_t dist_p(int x1, int y1, int x2, int y2) {
  const size_t x_gap = x1 - x2;
  const size_t y_gap = y1 - y2;
  return sqrt ((x_gap * x_gap) + (y_gap * y_gap));
}

static xc_gesture parse_gesture(unsigned int num_coords, TKCoord* coords, TKRect r) {
  
  size_t distance = 0;
  size_t farthest = 0;
  size_t farthest_dst = 0;
  int ends_in_region = 0;
  int mid_in_region = 0;
  
  distance = dist_p(coords[0].x, coords[0].y, coords[num_coords].x, coords[num_coords].y);
  
  for (int i = 0; i < num_coords ; i++) {
    size_t dst = dist_p(coords[0].x, coords[0].y, coords[i].x, coords[i].y);
    if (dst > farthest_dst) {
      farthest_dst = dst;
      farthest = i;
    }
  };
  
  ends_in_region = (parse_coord(coords[0], r) == parse_coord(coords[num_coords], r));
  mid_in_region = (parse_coord(coords[0], r) == parse_coord(coords[farthest], r));
  
  if (ends_in_region && mid_in_region && abs(distance - farthest_dst) > 8) {
    return XC_O;
  }
  
  if ((ends_in_region || abs(distance - farthest_dst) > 6 )&& !mid_in_region) {
    TKCoord* first = coords;
    TKCoord* last = &coords[farthest];
    
    float dx = first->x - last->x;
    float dy = first->y - last->y;
    
    int vertical = 0;
    int horizontal = 0;
    
    if (dx > 20) {horizontal = -1;} else if (dx < -20 ){ horizontal =1;}
    if (dy > 20) {vertical = -1;} else if (dy < -20){ vertical =1;}
    
    if (vertical==1 && horizontal==1 ) {return XC_SE_;}
    if (vertical==0 && horizontal==1 ) {return XC_E_;}
    if (vertical==-1 && horizontal==1 ) {return XC_NE_;}
    if (vertical==1 && horizontal==0 ) {return XC_S_;}
    if (vertical==1 && horizontal==-1 ) {return XC_SW_;}
    if (vertical==0 && horizontal==-1 ) {return XC_W_;}
    if (vertical==-1 && horizontal==-1 ) {return XC_NW_;}
    if (vertical==-1 && horizontal==0) {return XC_N_;}
    
    printf("fallback\n");
    
    return XC_X;
  }
  
  if (!ends_in_region) {
    TKCoord* first = coords;
    TKCoord* last = &coords[num_coords];
    
    float dx = first->x - last->x;
    float dy = first->y - last->y;
    
    int vertical = 0;
    int horizontal = 0;
    
    if (dx > 16) {horizontal = -1;} else if (dx < -20 ){ horizontal =1;}
    if (dy > 16) {vertical = -1;} else if (dy < -20){ vertical =1;}
    
    if (vertical==1 && horizontal==1 ) {return XC_SE;}
    if (vertical==0 && horizontal==1 ) {return XC_E;}
    if (vertical==-1 && horizontal==1 ) {return XC_NE;}
    if (vertical==1 && horizontal==0 ) {return XC_S;}
    if (vertical==1 && horizontal==-1 ) {return XC_SW;}
    if (vertical==0 && horizontal==-1 ) {return XC_W;}
    if (vertical==-1 && horizontal==-1 ) {return XC_NW;}
    if (vertical==-1 && horizontal==0) {return XC_N;}
    
    printf("fallback\n");
    
    return XC_X;
  }
  
  return XC_X;
}

static int composite_keycode(xc_region r, xc_gesture g) {
  
  if (g == XC_O) {
    switch (r) {
    case XC_1: return 'A';
    case XC_2: return 'N';
    case XC_3: return 'I';
    case XC_4: return 'H';
    case XC_5: return 'O';
    case XC_6: return 'R';
    case XC_7: return 'T';
    case XC_8: return 'E';
    case XC_9: return 'S';
    case XC_SPACE: return ' ';
    case XC_DELETE: return '\b';
    case XC_NOOP: return 0;
    }
  }
  
  if (g == XC_X) {
    switch (r) {
    case XC_1: return 'a';
    case XC_2: return 'n';
    case XC_3: return 'i';
    case XC_4: return 'h';
    case XC_5: return 'o';
    case XC_6: return 'r';
    case XC_7: return 't';
    case XC_8: return 'e';
    case XC_9: return 's';
    case XC_SPACE: return ' ';
    case XC_DELETE: return '\b';
    case XC_NOOP: return 0;
    }
  }
  
  if (r == XC_5) {
    switch (g) {
    case XC_NW: return 'q';
    case XC_N: return 'u';
    case XC_NE: return 'p';
    case XC_W: return 'c';
    case XC_E: return 'b';
    case XC_SW: return 'g';
    case XC_S: return 'd';
    case XC_SE: return 'j';
    case XC_O: return 'O';
    case XC_X: return 'o';
      
    case XC_NW_: return 'Q';
    case XC_N_: return 'U';
    case XC_NE_: return 'P';
    case XC_W_: return 'C';
    case XC_E_: return 'B';
    case XC_SW_: return 'G';
    case XC_S_: return 'D';
    case XC_SE_: return 'J';
    }
  }
  
  if (r == XC_1 && g == XC_SE) {return 'v';}
  if (r == XC_2 && g == XC_S) {return 'l';}
  if (r == XC_3 && g == XC_SW) {return 'x';}
  if (r == XC_4 && g == XC_E) {return 'k';}
  if (r == XC_6 && g == XC_W) {return 'm';}
  if (r == XC_7 && g == XC_NE) {return 'y';}
  if (r == XC_8 && g == XC_N) {return 'w';}
  if (r == XC_9 && g == XC_NW) {return 'f';}
  if (r == XC_8 && g == XC_E) {return 'z';}
  
  if (r == XC_1 && g == XC_SE_) {return 'V';}
  if (r == XC_2 && g == XC_S_) {return 'L';}
  if (r == XC_3 && g == XC_SW_) {return 'X';}
  if (r == XC_4 && g == XC_E_) {return 'K';}
  if (r == XC_6 && g == XC_W_) {return 'M';}
  if (r == XC_7 && g == XC_NE_) {return 'Y';}
  if (r == XC_8 && g == XC_N_) {return 'W';}
  if (r == XC_9 && g == XC_NW_) {return 'F';}
  if (r == XC_8 && g == XC_E_) {return 'Z';}
  
  
  return 0; // noop
}

static void print_state(xc_state state) {
  switch (state) {
  case XC_GESTURE_NONE: {printf("NONE\n"); break;}
  case XC_GESTURE_START: {printf("START\n");break;}
  case XC_GESTURE_CONTINUE: {printf("CONTINUE\n");break;}
  case XC_GESTURE_END: {printf("END\n");break;}
  }
}

int main() {
  
  TKWindowSystem* ws = tk_init_windowing();
  
  TKWindow* w = tk_create_window(ws,300,300,"XCunei");
  
  tk_show_window(ws, w);
  
  int running = 1;
  TKEvent* ev;
  
  TKColor col = {0,0,0};
  TKContext* paint = tk_create_context(ws, w, col, col, 1);
  TKColor red = {255,0,0};
  TKContext* stroke = tk_create_context(ws, w, red, red, 6);
  
  xc_state state = XC_GESTURE_NONE;
  
  TKCoord coords[200];
  unsigned int previous = 0;
  
  while(running == 1) {
    
    ev = tk_wait_for_next_event(ws);
    
    
    switch (ev->event_type) {
    case TK_FATAL_EV: {
      running = 0;
      break;
    }
    case TK_KEYBOARD_EV: {
      break;
    }
    case TK_MESSAGE_EV: {
      break;
    }
    case TK_MOUSE_ENTER_EV:
    case TK_MOUSE_LEAVE_EV: {
      if (state == XC_GESTURE_CONTINUE) {
        state = XC_GESTURE_NONE;
        tk_clear_window(ws, w);
        xc_draw_keyboard(ws, paint, w);
        previous = 0;
        break;
      }
      
      case TK_MOUSE_MOTION_EV: {
        
        if (ev->buttons & TK_MOUSE_1 && ev->b_state == 0 && state == XC_GESTURE_CONTINUE) {
          tk_draw_segment(ws, w, stroke, coords[previous], ev->coord);
          previous++;
          
          if (previous == 199) {
            previous = 0;
            coords[previous] = ev->coord;
            tk_clear_window(ws, w);
            xc_draw_keyboard(ws, paint, w);
            state = XC_GESTURE_START;
          } else {
            unsigned int dx = coords[previous].x - ev->coord.x;
            unsigned int dy = coords[previous].y - ev->coord.y;
            if (dx > 1 && dy > 1) {
              coords[previous] = ev->coord;
            } else {
              previous--;
            }
          }
          break;
        }
        
        if (ev->b_state == 1 && state == XC_GESTURE_NONE) {
          state = XC_GESTURE_START;
          coords[previous] = ev->coord;
          break;
        }
        
        if (ev->b_state == -1 && state == XC_GESTURE_START) {
          state = XC_GESTURE_NONE;
          TKRect window_region = tk_window_region(ws, w);
          xc_region r = parse_coord(coords[0], window_region);
          previous = 0;
          unsigned int keycode = composite_keycode(r, XC_X);
          if (keycode != 0) {
            tk_emit_key_event(ws, keycode);
          }
          break;
        }
        
        if (ev->buttons & TK_MOUSE_1 && state == XC_GESTURE_START) {
          state = XC_GESTURE_CONTINUE;
          break;
        }
        
        if (ev->b_state == -1 && state == XC_GESTURE_CONTINUE) {
          state = XC_GESTURE_END;
          TKRect window_region = tk_window_region(ws, w);
          xc_gesture g = parse_gesture(previous, coords, window_region);
          xc_region r = parse_coord(coords[0], window_region);
          unsigned int keycode = composite_keycode(r, g);
          if (keycode != 0) {
            tk_emit_key_event(ws, keycode);
          }
          
          tk_clear_window(ws, w);
          xc_draw_keyboard(ws, paint, w);
          previous = 0;
          break;
        }
        
        if (ev->buttons ^ TK_MOUSE_1 && state == XC_GESTURE_START) {
          state = XC_GESTURE_END;
          break;
        }
        
        if (ev->buttons ^ TK_MOUSE_1 && state == XC_GESTURE_END) {
          state = XC_GESTURE_NONE;
          break;
        }
        
        break;
      }
        case TK_WINDOW_EV: {
          xc_draw_keyboard(ws, paint, w);
          break;
        }
          case TK_UNKNOWN_EV: {
            break;
          }
    }
      free(ev);
    }
  }
  
  tk_destroy_window(ws, w);
  
  tk_deinit_windowing(ws);
  
  exit(EXIT_SUCCESS);
}
