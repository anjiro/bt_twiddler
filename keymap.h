#ifndef __KEYMAP_H__
#define __KEYMAP_H__

//thumb keys; ALT/CTRL/SHIFT defined according to USB HID spec
#define NUM   1 << 15
#define ALT   4
#define CTRL  1
#define SHIFT 2

#define A     1 << 11
#define B     1 << 10
#define C     1 <<  9
#define D     1 <<  8

#define E     1 <<  7
#define F     1 <<  6
#define G     1 <<  5
#define H     1 <<  4

#define SP    1 <<  3
#define TAB   1 <<  2
#define BS    1 <<  1
#define ENT   1 <<  0

//special keys
#define K_ARROW_DOWN  1
#define K_ARROW_UP    2
#define K_ARROW_LEFT  3
#define K_ARROW_RIGHT 4
#define K_PG_UP       5
#define K_PG_DN       6
#define K_ESC         7


struct key
{
  unsigned int keycode;
  char *str;
};

struct usb_key
{
  unsigned int key;
  unsigned int keycode;
};

extern struct key keymap[];
char *get_keymap_string(unsigned int keycode);
unsigned int convert_char_to_scancode(char c);
unsigned int convert_char_to_shift_scancode(char c);

#endif
