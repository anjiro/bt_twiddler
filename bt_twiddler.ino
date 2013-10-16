#include <Keypad.h>
#include "kprintf.h"
#include "keymap.h"
#include <usb_keyboard.h>


#define ROWS 4            //four rows
#define COLS 3            //three columns
char keys[ROWS][COLS] =
{
  {8,   4,  0},
  {9,   5,  1},
  {10,  6,  2},
  {11,  7,  3}
};
byte rowPins[ROWS] = { 0, 1, 2, 3 };  //connect to the row pinouts of the kpd
byte colPins[COLS] = { 4, 5, 6 };  //connect to the column pinouts of the kpd
unsigned int key_to_key[] = {A, B, C, D, E, F, G, H, SP, TAB, BS, ENT};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#define THUMB_ROWS 1
#define THUMB_COLS 4
char thumb_keys[THUMB_ROWS][THUMB_COLS] =
{
  {0, 1, 2, 3}
};
byte thumbRowPins[THUMB_ROWS] = {18};
byte thumbColPins[THUMB_COLS] = {17, 16, 15, 14};
unsigned int key_to_thumb[] = {NUM, ALT, CTRL, SHIFT};
Keypad thumb_kpd = Keypad(makeKeymap(thumb_keys), thumbRowPins, thumbColPins, THUMB_ROWS, THUMB_COLS);

unsigned long loopCount;
unsigned long startTime;
String msg;


void setup()
{
  Serial.begin(9600);
  Serial2.begin(115200);
  startTime = millis();
  pinMode(13, OUTPUT);
}


unsigned long t = 0;   //Last time the LED was lit
int b = 1;             //Value of the LED
void loop()
{
  char *key;
  unsigned int kval;
  unsigned int val = 0;
  int was_chord = 0, i;
  
  if(millis() - t > 100)
  {
    digitalWrite(13, b);
    b = !b;
    t = millis();
  }
  
  //Print output from the WT12 module
  while(Serial2.available())
    Serial.print((char)Serial2.read());
    
  //Pass input to WT12
  while(Serial.available())
    Serial2.print((char)Serial.read());
  
  if(thumb_kpd.getKeys())
    for(i = 0; i < LIST_MAX; i++)
      if(thumb_kpd.key[i].stateChanged)
        thumbKeypadEvent(key_to_thumb[thumb_kpd.key[i].kchar], thumb_kpd.key[i].kstate);
  
  if(kpd.getKeys())
    for(i = 0; i < LIST_MAX; i++)
      if(kpd.key[i].stateChanged)
        keypadEvent(key_to_key[kpd.key[i].kchar], kpd.key[i].kstate);
}


unsigned int key_modifier = 0;
void thumbKeypadEvent(unsigned int kval, int state)
{
  if(state != PRESSED && state != RELEASED)
    return;
    
  if(kval == NUM)
    return keypadEvent(kval, state);
  
  switch(state)
  {
    case PRESSED:
      key_modifier |= kval;
      break;
    
    case RELEASED:
      key_modifier ^= kval;
      break;
      
    default:
      break;
  }
  /*
  char *output_string = get_keymap_string(kval);
  kprintf("Modifier: %s ", output_string);
  if(state == PRESSED)
    Serial.println("down");
  else if(state == RELEASED)
    Serial.println("up");*/
}

unsigned int chord = 0;
int is_chord = 0;
void keypadEvent(unsigned int kval, int state)
{
  if(state != PRESSED && state != RELEASED)
    return;
    
  //char *keys = get_keymap_key(kval);
  char *output_string;
 
  switch(state)
  {
    case PRESSED:
      if(chord > 0)  //a key was already pressed
        is_chord = 1;
      chord |= kval;
      break;
      
    case RELEASED:
      output_string = get_keymap_string(chord);
      chord ^= kval;
      if(chord == 0)  //No more keys pressed
      {
        if(!is_chord)  //This was not the last key of a chord, so print it
        {
          Serial.print(output_string);
          send_string(output_string);
        }
        is_chord = 0;
      }
      else  //More keys held down, so print the current chord
      {
        Serial.print(output_string);
          send_string(output_string);
      }
      break;

    default:
      break;
  }
}

void send_string(char *s)
{
  kprintf("send_string(%s)\n", s);
  for(int i = 0; i < strlen(s); i++)
    send_key(s[i]);
}


void send_key(char k)
{
  //             0      1     2    3     4     5      6     7    8     9     10    11
  //                                    mod    0     kb0   kb1   kb2   kb3   kb4   kb5
  uint8_t seq[] = {0x9f, 0x0a, 0xa1, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  seq[4] = key_modifier;
  seq[6] = convert_char_to_scancode(k);
  if(seq[6] == 0)
  {
    seq[6] = convert_char_to_shift_scancode(k);
    if(seq[6] == 0)
      kprintf("\nScancode failure, can't find char %c (%d) in usb_keymap.\n", k, k);
    else
      seq[4] |= SHIFT;
  }
  for(int i = 0; i < 12; i++)
    Serial2.write(seq[i]);
  seq[6] = 0;
  for(int i = 0; i < 12; i++)
    Serial2.write(seq[i]);
}
