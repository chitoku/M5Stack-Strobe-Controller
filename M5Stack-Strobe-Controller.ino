/*
*******************************************************************************
* Built for M5Stack Core
*******************************************************************************
*/
#include <M5Stack.h>

int ledPin = 5;
int flash_in_us = 500;

const char MODE_SET_INTERVAL = 0;
const char MODE_SET_FLASHTIME = 1;
char mode = MODE_SET_INTERVAL;  

uint64_t  interval = 100000; // in micro-second (us)
uint64_t  interval_disp = 100000;
uint64_t  interval_min = 1000;
String strInterval;
int interval_digits = 6; // max to 999999 us --> 1 Hz
int interval_cursor_pos = interval_digits - 1;

float freq_in_hz;
int hz_x = 10;
int hz_y = 10;
int hz_unit_x = 230;
int hz_unit_y = 10;
float rpm;
int rpm_x = 10;
int rpm_y = 80;
int rpm_unit_x = 230;
int rpm_unit_y = 80;
int interval_x = 0;
int interval_y = 150;
int interval_number_x;
int cursor_y = 190;

int flashtime = 100; // in micro-second (us). 100us means 1/10 duty-cycle at 1000Hz
int flashtime_max = interval;
int flashtime_min = 5;
String strFlashtime;
int flashtime_digits = 4; // max to 9999 us --> 10 ms (100Hz max)

volatile int interruptintervaler;
int totalInterruptintervaler;
 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptintervaler++;
  portEXIT_CRITICAL_ISR(&timerMux);
 
}

void updateTimer(){
  timerAlarmDisable(timer);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, interval, true);
  timerAlarmEnable(timer);
}

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

void switch_mode(){
  Serial.println("### swtich_mode()");
  if(mode == MODE_SET_INTERVAL){
    Serial.println("---> MODE_SET_FLASHTIME");
    mode = MODE_SET_FLASHTIME;
    drawFlashtimeFrame();
    Serial.println("out from drawFlashtimeFrame()");
    drawFlashtime();
    Serial.println("out from drawFlashtime()");
  }else if(mode == MODE_SET_FLASHTIME){
    Serial.println("---> MODE_SET_INTERVAL");
    mode = MODE_SET_INTERVAL;
    drawIntervalFrame();
    drawInterval();
  }
  drawFooter();
}

void drawCursor(){
  for (int i=0; i < interval_digits; i++){
    if(i == interval_cursor_pos){
      M5.Lcd.fillRect(interval_number_x + i*24, cursor_y, 20, 10, CYAN);
    }else{
      M5.Lcd.fillRect(interval_number_x + i*24, cursor_y, 20, 10, BLUE);
    }
  }
}

void drawFreq(){
  freq_in_hz = 1000000.00 / interval;
  rpm = freq_in_hz * 60;
  String strFreq = String(freq_in_hz,2);
  String strRpm = String(rpm,1);
  M5.Lcd.setTextSize(5);
  int x = hz_unit_x - 10 - M5.Lcd.textWidth(strFreq);
  M5.Lcd.fillRect(0, hz_y, hz_unit_x, rpm_unit_y - hz_unit_y, BLACK); // TODO: Optimize to only fill the changed digit
  M5.Lcd.setCursor(x, hz_y);
  M5.Lcd.setTextColor(LIGHTGREY);
  M5.Lcd.println(strFreq);
  x = rpm_unit_x - 10 - M5.Lcd.textWidth(strRpm);
  M5.Lcd.fillRect(0, rpm_y, rpm_unit_x, interval_y - rpm_unit_y, BLACK); // TODO: Optimize to only fill the changed digit
  M5.Lcd.setCursor(x, rpm_y);
  M5.Lcd.setTextColor(LIGHTGREY);
  M5.Lcd.println(strRpm);
  
  Serial.print("Freq: ");
  Serial.print(strFreq);
  Serial.print("  RPM: ");
  Serial.println(strRpm);
}

void drawIntervalFrame(){
  M5.Lcd.fillRect(0, interval_y, 320, 70, BLACK); 
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(interval_x, interval_y);
  M5.Lcd.print("Interval ");
  interval_number_x = M5.Lcd.getCursorX();
  M5.Lcd.setCursor(interval_x + 50, interval_y + 18);
  M5.Lcd.print("(us)");

  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(interval_number_x, interval_y);
  String strInterval = uint64ToString(interval);
  int w = M5.Lcd.textWidth(strInterval);
  M5.Lcd.print(strInterval);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(M5.Lcd.getCursorX()+28, M5.Lcd.getCursorY()+10);
  M5.Lcd.print("us");
  interval_disp = interval;

  int x = interval_number_x + w + 2;
  int y = interval_y + 4;
  M5.Lcd.fillTriangle(x+8, y+0,    x, y+8,     x+16, y+8,  RED);
  M5.Lcd.fillTriangle(x+8, y+24,   x, y+16,    x+16, y+16, GREEN);
}

void drawInterval(){
  M5.Lcd.fillRect(interval_number_x, interval_y, 24*interval_digits, 40, BLACK); // TODO: Optimize to only fill the changed digit
  M5.Lcd.setCursor(interval_number_x, interval_y);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(4);
  strInterval = uint64ToString(interval);
  while(strInterval.length() < interval_digits){
    strInterval = "0" + strInterval;
  }
  M5.Lcd.println(strInterval);
  interval_disp = interval;
}

void drawFlashtimeFrame(){
  M5.Lcd.fillRect(0, interval_y, 320, 70, BLACK); 
  M5.Lcd.setTextColor(PINK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(interval_x, interval_y);
  M5.Lcd.println(" Flash ");
  M5.Lcd.setCursor(interval_x, interval_y + 18);
  M5.Lcd.println("duration");

  M5.Lcd.fillRect(interval_number_x, cursor_y, 20 + 24*flashtime_digits, 10, BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(interval_number_x, interval_y);
  String strFlashtime = uint64ToString(flashtime);
  while(strFlashtime.length() < flashtime_digits){
    strFlashtime = " " + strFlashtime;
  }
  int w = M5.Lcd.textWidth(strFlashtime);
  M5.Lcd.print(strFlashtime);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(M5.Lcd.getCursorX()+28, M5.Lcd.getCursorY()+10);
  M5.Lcd.print("us");

  int x = interval_number_x + 24*flashtime_digits + 2;
  int y = interval_y + 4;
  M5.Lcd.fillTriangle(x+8, y+0,    x, y+8,     x+16, y+8,  RED);
  M5.Lcd.fillTriangle(x+8, y+24,   x, y+16,    x+16, y+16, GREEN);
}

void drawFlashtime(){
  M5.Lcd.fillRect(interval_number_x, interval_y, 24*flashtime_digits, 40, BLACK); // TODO: Optimize to only fill the changed digit
  M5.Lcd.setCursor(interval_number_x, interval_y);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(4);
  strFlashtime = uint64ToString(flashtime);
  Serial.print(strFlashtime);
  while(strFlashtime.length() < flashtime_digits){
    strFlashtime = " " + strFlashtime;
  }
  M5.Lcd.println(strFlashtime);
}

void drawFooter(){
  M5.Lcd.fillRoundRect( 10,220,  80,20,   5,   RED);
  M5.Lcd.fillRoundRect(230,220,  80,20,   5,   GREEN);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setCursor(45,224);
  M5.Lcd.println("+");
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setCursor(265,224);
  M5.Lcd.println("-");

  if(mode == MODE_SET_INTERVAL){
    M5.Lcd.fillRoundRect(100,215, 120,30,  10,   PINK);
    M5.Lcd.fillRoundRect(105,220, 110,20,   5,   CYAN);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setCursor(113,223);
    M5.Lcd.println("Cursor>>");
  }else if(mode == MODE_SET_FLASHTIME){
    M5.Lcd.fillRoundRect(100,215, 120,30,  10,   YELLOW);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setCursor(113,223);
    M5.Lcd.println("<-Intval");
  }
}

/* After M5Core is started or reset
  the program in the setUp () function will be run, and this part will only be run once.*/
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  M5.begin();  //Init M5Core.
  M5.Power.begin(); //Init Power module. 
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.setTextSize(5);
  M5.Lcd.setCursor(hz_unit_x, hz_unit_y);
  M5.Lcd.println("Hz");
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.setCursor(rpm_unit_x, rpm_unit_y);
  M5.Lcd.println("rpm");

  drawFreq();
  drawIntervalFrame();
  drawInterval();
  drawCursor();

  drawFooter();
  
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, interval, true);
  timerAlarmEnable(timer);
}

/* After the program in setup() runs, it runs the program in loop()
The loop() function is an infinite loop in which the program runs repeatedly*/
void loop() {

  if (interruptintervaler > 0) {
 
    portENTER_CRITICAL(&timerMux);
    interruptintervaler--;
    portEXIT_CRITICAL(&timerMux);
 
    totalInterruptintervaler++;

    digitalWrite(ledPin, HIGH);
    delayMicroseconds(flashtime);
    digitalWrite(ledPin, LOW);
 
    Serial.print("Interrupt: ");
    Serial.println(totalInterruptintervaler);
 
  }
  
  M5.update(); //Read the press state of the key.  
  
  switch(mode){
  case MODE_SET_INTERVAL:
    if (M5.BtnB.pressedFor(1000,2000)){
      Serial.println(" --> SET_FLASHTIME mode");
      switch_mode();
    }else if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 10)) {
      interval += pow(10,interval_digits-1 - interval_cursor_pos);
      if(interval >= pow(10,interval_digits)){
        interval = pow(10,interval_digits)-1;
      }
      updateTimer();
      drawInterval();
      drawFreq();
    } else if (M5.BtnB.wasReleased()) {
      interval_cursor_pos++;
      if(interval_cursor_pos == interval_digits){
        interval_cursor_pos = 0;
      }
      drawCursor();
    } else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 10)) {
        if(interval <= pow(10,interval_digits-1-interval_cursor_pos)){
          interval = interval_min;
        }else{
          interval -= pow(10,interval_digits-1-interval_cursor_pos);
        }
        if(interval < interval_min){
          interval = interval_min;
        }
        updateTimer();
        drawInterval();
        drawFreq();
    } 
    break;    
    
  case MODE_SET_FLASHTIME:
    if (M5.BtnB.pressedFor(1000,2000)){
      switch_mode();
    }else if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 10)) {
      flashtime++;
      if(flashtime >= interval){
        flashtime = interval;
      }
      drawFlashtime();
      updateTimer();
    } else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 10)) {
      flashtime--;
      if(flashtime <= flashtime_min){
        flashtime = flashtime_min;
      }
      drawFlashtime();
      updateTimer();
    }
    break;
  }
}
