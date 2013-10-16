#include <Arduino.h>
#include <stdarg.h>

int kprintf(char *format, ...)  {
  int n;
  static char buf[1024];
  va_list args;
  va_start (args, format);
  vsnprintf(buf, sizeof(buf), format, args); // does not overrun sizeof(buf) including null terminator
  va_end (args);
  // the below assumes that the new data will fit into the I/O buffer. If not, Serial may drop it.
  // if Serial had a get free buffer count, we could delay and retry. Such does exist at the device class level, but not at this level.
  n = strlen(buf) - Serial.print(buf); // move chars to I/O buffer, freeing up local buf
  return n; // number of chars unable to fit in device I/O buffer (see bug notice above)
}
