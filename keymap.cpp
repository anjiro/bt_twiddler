#include "keymap.h"
#include "kprintf.h"

struct key keymap[] =
{
  {A,  "a"},
  {B,  "b"},
  {C,  "c"},
  {D,  "d"},
  {E,  "e"},
  {F,  "f"},
  {G,  "g"},
  {H,  "h"},
  {SP,  " "},
  {TAB,  "\t"},
  {BS,  "\b"},
  {ENT,  "\n"},
  
  {SP | B,   "i"},
  {SP | C,   "j"},
  {SP | D,   "k"},
  {SP | F,   "l"},
  {SP | G,   "m"},
  {SP | H,   "n"},
  {SP | TAB, "o"},
  {SP | BS,  "p"},
  {SP | ENT, "q"},
  
  {E | B,    "r"},
  {E | C,    "s"},
  {E | D,    "t"},
  {E | F,    "u"},
  {E | G,    "v"},
  {E | H,    "w"},
  {E | TAB,  "x"},
  {E | BS,   "y"},
  {E | ENT,  "z"},
  
  {A | B, "."},
  {A | C | D, "ant "},
  {A | C, ","},
  {A | D, ";"},
  {A | F, "'"},
  {A | G, "\""},
  {A | H, "and "},
  {A | TAB,   "?"},
  {A | BS,    "!"},
  {A | ENT,   "-"},

  {B | BS,    ")"},
  {B | C,     "the "},
  {B | D,     "to "},
  {B | ENT,   "_"},
  {B | G,     "of "},
  {B | H,     "in"},

  {C | ENT,   ":"},
  {C | H,     "%"},
  {E | F | G, "/"},
  {E | G | H, "ent "},
  {F | D,     "ed "},
  {F | ENT,   "#"},
  {G | D,     "="},
  {G | ENT,   "+"},
  {G | H,     "ing "},
  {SP | TAB | BS, "\\"},
  {TAB | BS,  "("},
  {TAB | C,   "$"},
  {TAB | ENT, "&"},
  {TAB | H,   "ion "},
  {BS | D,    "@"},
  {BS | H,    "*"},
  
  {A | B | C, (char[]){K_ESC, 0}},
  {F | H,     (char[]){K_ARROW_DOWN, 0}},
  {F | G,     (char[]){K_ARROW_UP, 0}},
  {F | C,     (char[]){K_ARROW_LEFT, 0}},
  {F | BS,    (char[]){K_ARROW_RIGHT, 0}},
  {BS | ENT,  (char[]){K_PG_UP, 0}},
  {C | D,     (char[]){K_PG_DN, 0}},
  
  {NUM | A, "1"},
  {NUM | E, "2"},
  {NUM | SP, "3"},
  {NUM | B, "4"},
  {NUM | F, "5"},
  {NUM | TAB, "6"},
  {NUM | C, "7"},
  {NUM | G, "8"},
  {NUM | BS, "9"},
  {NUM | H, "0"},
};


struct usb_key usb_keymap[] =
{
  {'a',	0x04},
  {'b',	0x05},
  {'c',	0x06},
  {'d',	0x07},
  {'e',	0x08},
  {'f',	0x09},
  {'g',	0x0A},
  {'h',	0x0B},
  {'i',	0x0C},
  {'j',	0x0D},
  {'k',	0x0E},
  {'l',	0x0F},
  {'m',	0x10},
  {'n',	0x11},
  {'o',	0x12},
  {'p',	0x13},
  {'q',	0x14},
  {'r',	0x15},
  {'s',	0x16},
  {'t',	0x17},
  {'u',	0x18},
  {'v',	0x19},
  {'w',	0x1A},
  {'x',	0x1B},
  {'y',	0x1C},
  {'z',	0x1D},
  {'1',	0x1E},
  {'2',	0x1F},
  {'3',	0x20},
  {'4',	0x21},
  {'5',	0x22},
  {'6',	0x23},
  {'7',	0x24},
  {'8',	0x25},
  {'9',	0x26},
  {'0',	0x27},
  {'\n',0x28},
  {K_ESC,0x29},
  {'\b',0x2A},
  {'\t',0x2B},
  {' ',	0x2C},
  {'-',	0x2D},
  {'=',	0x2E},
  {'[',	0x2F},
  {']',	0x30},
  {'\\',0x31},
  {';',	0x33},
  {'\'',0x34},
  {'`', 0x35},
  {',',	0x36},
  {'.',	0x37},
  {'/',	0x38},
  {K_PG_UP,	0x4B},
  {K_PG_DN,	0x4E},
  {K_ARROW_RIGHT,	0x4F},
  {K_ARROW_LEFT,	0x50},
  {K_ARROW_DOWN,	0x51},
  {K_ARROW_UP,	0x52},
  {CTRL,  0xE0},
  {SHIFT, 0xE1},
  {ALT,   0xE2},  
};


struct usb_key shift_usb_keymap[] =
{
  {'!', 0x1E},
  {'@', 0x1F},
  {'#', 0x20},
  {'$', 0x21},
  {'%', 0x22},
  {'^', 0x23},
  {'&', 0x24},
  {'*', 0x25},
  {'(', 0x26},
  {')', 0x27},
  {'_', 0x2D},
  {'+', 0x2E},
  {'{', 0x2F},
  {'}', 0x30},
  {'|', 0x31},
  {'"', 0x34},
  {'~', 0x35},
  {'<', 0x36},
  {'>', 0x37},
  {'?', 0x38},
};


//Get the user-defined string from the keymap based on the pressed keys
char *get_keymap_string(unsigned int keycode)
{
  int i;
  for(i = 0; i < sizeof(keymap); i++)
    if(keymap[i].keycode == keycode)
      return keymap[i].str;
  return 0;
}


//Convert a given character to the USB keycode
unsigned int convert_char_to_scancode(char c)
{
  for(int i = 0; i < sizeof(usb_keymap); i++)
    if(usb_keymap[i].key == c)
      return usb_keymap[i].keycode;
  return 0;
}


//Convert a given character to the USB keycode, assuming shift pressed
unsigned int convert_char_to_shift_scancode(char c)
{
  for(int i = 0; i < sizeof(shift_usb_keymap); i++)
    if(shift_usb_keymap[i].key == c)
      return shift_usb_keymap[i].keycode;
  return 0;
}
