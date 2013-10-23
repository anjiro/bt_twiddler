#include <Keypad.h>
#include "kprintf.h"
#include "keymap.h"
#include <usb_keyboard.h>
#include <i2c_t3.h>
#include <MAX1704.h>

#define BT_CMD_PIN 15

#define ROWS 4            //four rows
#define COLS 3            //three columns
char keys[ROWS][COLS] =
{
  {3, 7, 11},
  {2, 6, 10},
  {1, 5,  9},
  {0, 4,  8}
};
byte rowPins[ROWS] = { 0, 1, 2, 3 };  //connect to the row pinouts of the kpd
byte colPins[COLS] = { 4, 5, 6 };     //connect to the column pinouts of the kpd
unsigned int key_to_key[] = {A, B, C, D, E, F, G, H, SP, TAB, BS, ENT};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

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

unsigned long loopCount;
String msg;

MAX1704 fuelGauge;

int was_cmd_req = 0;   //Set to 1 when a command mode request was sent via BT_CMD_PIN

void setup()
{
  Wire.begin(I2C_MASTER, 0, I2C_PINS_16_17, I2C_PULLUP_INT, I2C_RATE_100);
  Serial.begin(9600);
  Serial3.begin(115200);
  pinMode(13, OUTPUT);
  
  fuelGauge.reset();
  fuelGauge.quickStart();
  fuelGauge.showConfig();
  
  //Set up the BT reset pin and reset it
  
  pinMode(BT_CMD_PIN, OUTPUT);
  //bt_reset();
  /*digitalWrite(BT_CMD_PIN, HIGH);
  delay(50);
  digitalWrite(BT_CMD_PIN, LOW);
  */
}


unsigned long t = 0;     //Last time the LED was lit
unsigned long t2 = 0;
int b = 1;               //Value of the LED
char iwrap_string[255];  //Keep track of strings coming from iWrap
int  str_pos = 0;        //Position in the iwrap string


void loop()
{
  char *key;
  char c;
  unsigned int kval;
  unsigned int val = 0;
  int was_chord = 0, i;
  
  if(millis() - t > 100)
  {
    digitalWrite(13, b);
    b = !b;
    t = millis();
  }
  
  if(millis() - t2 > 5000)
  {
    kprintf("Battery charge: %.2f%%\n", fuelGauge.stateOfCharge());
    t2 = millis();
  }
  
  //Print output from the WT12 module
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
    
  //Pass input to WT12
  while(Serial.available())
    Serial3.print((char)Serial.read());
  
  if(thumb_kpd.getKeys())
    for(i = 0; i < LIST_MAX; i++)
      if(thumb_kpd.key[i].stateChanged)
      {
        kprintf("Thumb: %d\n", thumb_kpd.key[i].kchar);
        thumbKeypadEvent(key_to_thumb[thumb_kpd.key[i].kchar], thumb_kpd.key[i].kstate);
      }
  
  if(kpd.getKeys())
    for(i = 0; i < LIST_MAX; i++)
      if(kpd.key[i].stateChanged)
        keypadEvent(key_to_key[kpd.key[i].kchar], kpd.key[i].kstate);
}


int iwrap_event(char *event)
{
  if(!strcmp(event, "READY."))
  {
    Serial.println("Detected 'READY.'");
    if(was_cmd_req)
    {
      was_cmd_req = 0;
      Serial3.println("RESET");
    }
    return 1;
  }
   else if(!strcmp(event, "CONNECT 0 HID 11"))
   {
     Serial.println("Detected connection; sending SELECT.");
     Serial3.println("SELECT 0");
     return 1;
   }
   return 0;
}


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
  
  //char *output_string = get_keymap_string(kval);
  //kprintf("Modifier: %s ", output_string);
  if(state == PRESSED)
    Serial.println("down");
  else if(state == RELEASED)
    Serial.println("up");
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
          send_string(output_string);
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
    Serial3.write(seq[i]);
  seq[6] = 0;
  for(int i = 0; i < 12; i++)
    Serial3.write(seq[i]);
}
