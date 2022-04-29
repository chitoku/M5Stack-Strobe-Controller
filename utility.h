#include <math.h>

String uint64ToString(uint64_t input) {
  String result = "";
  uint8_t base = 10;

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c +='0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

uint64_t floatToUint64(float f) {
    return (uint64_t) f;
}

uint64_t freqToInterval(float freq) {
    uint64_t interval = floatToUint64(1000000.0 / freq);
    return interval;
}

String floatToString(float f){
  String result = "";
  if (f >= 100){
    result = String(f, 0);
  }else{
    result = String(f, 1);
  }
  return result;
}

int usToLogPx(float f){
  int i = 3;
  i = (int)(( log10(f) - 2 ) * 75);
  return i;
}