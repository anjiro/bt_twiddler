#include <Keypad.h>
#include "kprintf.h"
#include "keymap.h"
#include <usb_keyboard.h>
#include <i2c_t3.h>
#include <MAX1704.h>
#include <SyncLED.h>

#define strncmp2(s1, s2) strncmp(s1, s2, sizeof(s2)-1)

//Periodically do stuff
MAX1704 fuelGauge;                     // Object for battery charge device
unsigned long last_battery_print = 0;  //Last time the battery level was printed
unsigned long last_battery_check = 0;  //Last time the battery level was checked
unsigned long last_blink_pattern = 0;  //Only update when it changes
SyncLED StatusLight(13);               //Blinking status light on pin 13

//Bluetooth communication
#define BT_CMD_PIN 15    //On transition to high, the WT12 enters command mode
char iwrap_string[255];  //Keep track of strings coming from iWrap
int  str_pos = 0;        //Position in the iwrap string
int  was_cmd_req = 0;    //Set to 1 when a command mode request was sent via BT_CMD_PIN
int  bt_connected = 0;   //Is the Bluetooth link connected? If not, don't try to send keypresses

//Keyboard variables
unsigned int key_modifier = 0;  //Shift, Alt, etc
unsigned int keys_down = 0;     //Keep track of keys pressed
int is_chord = 0;               //More than one key pressed?
int output_chord = 0;           //Sent keypress for the current chord?

//Set up front keypad
#define KEY_ROWS 4
#define KEY_COLS 3
char front_keys[KEY_ROWS][KEY_COLS] =
{
  {3, 7, 11},
  {2, 6, 10},
  {1, 5,  9},
  {0, 4,  8}
};
byte rowPins[KEY_ROWS] = {0, 1, 2, 3};
byte colPins[KEY_COLS] = {4, 5, 6};
unsigned int key_to_key[] = {A, B, C, D, E, F, G, H, SP, TAB, BS, ENT};
Keypad kpd = Keypad(makeKeymap(front_keys), rowPins, colPins, KEY_ROWS, KEY_COLS);

//Set up thumbpad
#define THUMB_ROWS 1
#define THUMB_COLS 4
char thumb_keys[THUMB_ROWS][THUMB_COLS] =
{
  {0, 1, 2, 3}
};
byte thumbRowPins[THUMB_ROWS] = {22};
byte thumbColPins[THUMB_COLS] = {21, 20, 19, 18};
unsigned int key_to_thumb[] = {NUM, ALT, CTRL, SHIFT};
Keypad thumb_kpd = Keypad(makeKeymap(thumb_keys), thumbRowPins, thumbColPins, THUMB_ROWS, THUMB_COLS);



void setup()
{
  Wire.begin(I2C_MASTER, 0, I2C_PINS_16_17, I2C_PULLUP_INT, I2C_RATE_100);
  Serial.begin(9600);
  Serial3.begin(115200);

  //Initial blink pattern
  StatusLight.blinkPattern(0b00110011UL, 50, 8);
  
  fuelGauge.reset();
  fuelGauge.quickStart();
  fuelGauge.showConfig();
  
  //Set up the BT reset pin and reset it
  pinMode(BT_CMD_PIN, OUTPUT);
  //bt_reset();
}


unsigned long t = 0;
int b = 0;
void loop()
{
  char *key;
  char c;
  unsigned int i;
  unsigned int val = 0;
  unsigned long blink_pattern = 0;
  
  StatusLight.update();

  if(millis() - last_battery_check > 250)
  {
    float charge = fuelGauge.stateOfCharge();

    if(millis() - last_battery_print > 20000)
    {
      kprintf("Battery charge: %.2f%%\n", charge);
      last_battery_print = millis();
    }
    
    //Blink 1-5 times based on percentage of charge
    if(charge > 80)
      blink_pattern = 0b1010101010000000UL;
    else if(charge > 60)
      blink_pattern = 0b1010101000000000UL;
    else if(charge > 40)
      blink_pattern = 0b1010100000000000UL;
    else if(charge > 20)
      blink_pattern = 0b1010000000000000UL;
    else
      blink_pattern = 0b1000000000000000UL;
    if(bt_connected)
      blink_pattern |= 0b0000000000111110;
    if(blink_pattern != last_blink_pattern)
    {
      StatusLight.blinkPattern(blink_pattern, 100, 16);
      last_blink_pattern = blink_pattern;
    }
  }
  
  //Print output from the WT12 module and watch for events
  while(Serial3.available())
  {
    c = (char)Serial3.read();
    Serial.print(c);
    if(c != '\r' && c != '\n')
      iwrap_string[str_pos++] = c;
    else if(c == '\r')
    {
      iwrap_string[str_pos++] = 0;
      iwrap_event(iwrap_string);
      str_pos = 0;
      iwrap_string[0] = 0;
    }
  }
    
  //Pass input from the serial port to WT12
  while(Serial.available())
    Serial3.print((char)Serial.read());
  
  //Check for thumb button presses
  if(thumb_kpd.getKeys())
    for(i = 0; i < LIST_MAX; i++)
      if(thumb_kpd.key[i].stateChanged)
      {
        //kprintf("Thumb: %d\n", thumb_kpd.key[i].kchar);
        thumbKeypadEvent(key_to_thumb[thumb_kpd.key[i].kchar], thumb_kpd.key[i].kstate);
      }
  
  //Check for keyboard button presses
  if(kpd.getKeys())
    for(i = 0; i < LIST_MAX; i++)
      if(kpd.key[i].stateChanged)
        keypadEvent(key_to_key[kpd.key[i].kchar], kpd.key[i].kstate);
}



//Got a string from the BT module; do something?
void iwrap_event(char *event)
{
  if(!strcmp(event, "READY."))
  {
    if(was_cmd_req)
    {
      was_cmd_req = 0;
      Serial3.println("RESET");
    }
  }
  else if(!strcmp(event, "CONNECT 0 HID 11") || !strncmp2(event, "RING 0"))
  {
    Serial3.println("SELECT 0");
    bt_connected = 1;
  }
  else if(!strncmp2(event, "CONNECT 0"))
    bt_connected = 1;
  else if(!strncmp2(event, "NO CARRIER 0"))
    bt_connected = 0;
}


//Output the battery level via the keyboard
void print_battery_level(unsigned int keycode)
{
  char battery_level[50];
  snprintf(battery_level, 50, "\nBattery level %.2f%%\n", fuelGauge.stateOfCharge());
  send_string(battery_level);
}



//Reset the BT connection
void bt_reset()
{
  kprintf("Reset BT chip\n");
  digitalWrite(BT_CMD_PIN, HIGH);
  delay(50);
  digitalWrite(BT_CMD_PIN, LOW);
  was_cmd_req = 1;
  kprintf("Done\n");
  return;
}



//When a thumb key is pressed
void thumbKeypadEvent(unsigned int kval, int state)
{
  if(state != PRESSED && state != RELEASED)
    return;
    
  kprintf("thumb: 0x%02X, %d\n", kval, state);
    
  //BUG: prevents NUM from being used as part of a chord
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
}



//Called when keypad keys are pressed
/* 
  Chord logic: do nothing until a key is released. If it was the only
  key, print its value. If more than one key was pressed:
    - if the released key is not the last one (i.e., at least one key is
      still held down), then it's a chord, so print it out
    - if the released key was the last one of several, then the chord
      value has already been printed out, so don't do anything else.
*/
void keypadEvent(unsigned int kval, int state)
{
  if(state != PRESSED && state != RELEASED)
    return;
    
  char *output_string;
  
  switch(state)
  {
    case PRESSED:
      //kprintf("Down: %d\n", kval);
      if(keys_down > 0)  //a key was already pressed
        is_chord = 1;
      keys_down |= kval;
      output_chord = 0;
      break;
      
    case RELEASED:
      output_string = get_keymap_string(keys_down);
      keys_down ^= kval;

      kprintf("Up: %d; keys_down = %d; is_chord = %d\n", kval, keys_down, is_chord);
      
      if(keys_down == 0)
      {
        if(!is_chord)
          send_string(output_string);
        output_chord = 0;
        is_chord = 0;
      }
      else if(!output_chord)
      {
        send_string(output_string);
        output_chord = 1;
      }
      break;

    default:
      break;
  }
}



void send_string(char *s)
{
  //kprintf("send_string(%s)\n", s);
  for(int i = 0; i < strlen(s); i++)
    send_key(s[i]);
}



void send_key(char k)
{
  kprintf("send_key(0x%02X)\n", k);
  if(!bt_connected)
    return;
    
  //                0      1     2    3     4     5      6     7    8     9     10    11
  //                                       mod    0     kb0   kb1   kb2   kb3   kb4   kb5
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
    Serial3.write(seq[i]);

  //Modify seq to remvoe sent key and drop shift if it was there
  seq[6] = 0;
  if(seq[4] & SHIFT)
    seq[4] ^= SHIFT;

  for(int i = 0; i < 12; i++)
    Serial3.write(seq[i]);
}
