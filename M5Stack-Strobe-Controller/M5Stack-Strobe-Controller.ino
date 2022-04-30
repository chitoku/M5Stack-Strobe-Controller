/*
*****************************************************************************
* Built for M5Stack Core
*****************************************************************************
*/
// @formatter:off
#include <M5Stack.h>
#include "utility.h"
#include <math.h>

int ledPin = 5;

const char MODE_SET_INTERVAL = 0;
const char MODE_SET_FLASHTIME = 1;
const char MODE_SET_FREQ = 2;
char mode = MODE_SET_FREQ;

float freq = 10.0;
float maxFreq = 1000.0;
float minFreq = 1.0;
String strFreq = floatToString(freq);

uint64_t interval = freqToInterval(freq);
String strInterval = uint64ToString(interval);

float rpm = freq * 60;
String strRpm = floatToString(rpm);

int flashtime = 200; // in micro-second (us). 0.2ms means 1/500 duty-cycle at 10Hz (100ms)
int maxFlashtime = interval;
int minFlashtime = 5;
String strFlashtime = String(flashtime);

/*

                        1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
   ┌────────────────────────────────────────────────────────────────┐-> [X]
  0│                                                                │
  1│      ┌                                      ┐                  │ (freqX, freqY) 
  2│         11    0000   0000   0000      0000                     │  (freqX2, freqY2) 
  3│     ◄    1    0  0   0  0   0  0      0  0   ►   ┌             │     (freqUnitX, freqUnitY)
  4│         111   0000   0000   0000  **  0000          H z        │  
  5│                                             ┘             ┘    │ 
  6│  ┌            ┌                      │  ┌                      │ (intvlTitleX, intvlTitleY)
  7│    Interval     1  0  0  0  ┌        │    1  0  0  0  ┌        │ (intvlX, intvlY)
  8│             ┘              ┘  u s ┘  │               ┘  rpm ┘  │ (intvlUnitX, intvlUnitY)
  9│                                                                │ (rpmX, rpmY) (rpmUnitX, rpmUnitY)
 10│ |<==========================================================>| │
 11│ ┌─────────────┐                                              ┌─┤ (graphTopY)
 12│ │             │                                              │ │ (graphRiseX1) (graphRiseX2)
 13│ │             │                                              │ │
 14│ │             │                                              │ │
 15├─┘             └──────────────────────────────────────────────┘ │ (graphBaseY)
 16│ |<----------->|                                              : │
 17│                                                                │ (flashTitleX, flashTitleY)
 18│ ┌                          ┌                                   │ (flashX, flashY)
 19│   Flash speed            ◄     1 /  1  0  0     ►   ┌          │  (flashX2, flashY2)
 20│                     ┘                         ┘       u s ┘    │ (flashUnitX, flashUnitY)
 21│                       ┌──────────────────┐                     │ 
 22│  ┌───────────────┐    │         ▲        │   ┌───────────────┐ │
 23│  │    ◄  +       │    │         ▼        │   │       -   ►   │ │
 24└──┴───────────────┴────┴──────────────────┴───┴───────────────┴─┘
   |  (10,220)             (100,215)             (230,220)
   V            (90,240)               (220,240)           (310,240)
  [Y]
*/

int fX = 0;                 int fY = 160; // base
int freqTitleX = 10 + fX;   int freqTitleY = 0 + fY;
int freqX = 70 + fX;        int freqY = 0 + fY;
int freqX2 = 250 + fX;      int freqY2 = 50 + fY;
int freqUnitX = 270 + fX;   int freqUnitY = 15 + fY;

int iX = 0;                 int iY = 130;
int intvlTitleX = 0 + iX;   int intvlTitleY = 0 + iY;
int intvlX = 91 + iX;       int intvlY = 0 + iY;
int intvlX2 = 175 + iX;     int intvlY2 = 25 + iY;
int intvlUnitX = 185 + iX;  int intvlUnitY = 0 + iY;
int rpmX = 210 + iX;        int rpmY = 0 + iY;
int rpmX2 = 280 + iX;       int rpmY2 = 25 + iY;
int rpmUnitX = 282 + iX;    int rpmUnitY = 0 + iY;

int gX = 0;                 int gY = 55; // base
int graphTopY = 0 + gY;     int graphBaseY = 40 + gY;
int graphRiseX1 = 10;       int graphRiseX2 = 310;

int sX = 10;                int sY = 10; // base
int flashTitleX = 0 + sX;   int flashTitleY = 0 + sY;
int flashX = 150 + sX;      int flashY = -5 + sY;
int flashX2 = 230 + sX;     int flashY2 = 15 + sY;
int flashUnitX = 260 + sX;  int flashUnitY = sY;

int w = 0;

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

void drawFreqFrame() {
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(freqTitleX, freqTitleY);
  M5.Lcd.println("Freq");
  
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(freqUnitX, freqUnitY);
  M5.Lcd.println("Hz");

  //M5.Lcd.fillRect(0, intvlTitleY-5, rpmX-intvlTitleX, intvlY2-intvlY,  PURPLE);
  M5.Lcd.setTextColor(PINK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(intvlTitleX, intvlTitleY);
  M5.Lcd.println("Interval");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(intvlUnitX, intvlUnitY);
  M5.Lcd.println("us");
  
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(rpmUnitX, rpmUnitY);
  M5.Lcd.println("rpm");
}

void drawFreq() {
  M5.Lcd.fillRect(freqX, freqY, freqX2-freqX, freqY2-freqY,  BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(6);
  w = M5.Lcd.textWidth(strFreq);
  M5.Lcd.setCursor(freqX2 - w, freqY);
  M5.Lcd.println(strFreq);

  M5.Lcd.fillRect(intvlX+5, intvlY-5, intvlX2-intvlX, intvlY2-intvlY,  PURPLE);
  M5.Lcd.setTextSize(2);
  w = M5.Lcd.textWidth(strInterval);
  M5.Lcd.setCursor(intvlX2-w, intvlY);
  M5.Lcd.println(strInterval);

  M5.Lcd.fillRect(rpmX, rpmY, rpmX2-rpmX, rpmY2-rpmY,  BLACK);
  M5.Lcd.setTextSize(2);
  w = M5.Lcd.textWidth(strRpm);
  M5.Lcd.setCursor(rpmX2-w, rpmY);
  M5.Lcd.println(strRpm);
}

void drawGraphFrame(){
  M5.Lcd.fillRect(0,graphBaseY, 320, 3,  YELLOW);
}

void drawGraph(){
  M5.Lcd.fillRect(0,graphTopY, 320, graphBaseY-graphTopY,  BLACK);
  int graphIntervalPx = usToLogPx(interval);
  int graphFlashPx = usToLogPx(flashtime);
  Serial.print(graphFlashPx);
  if (graphFlashPx == graphIntervalPx) { graphFlashPx--; }
  Serial.print(" --> ");
  Serial.println(graphFlashPx);
  for(int x=graphRiseX1; x<320; x+=graphIntervalPx){
    M5.Lcd.fillRect(x,graphTopY, graphFlashPx, graphBaseY-graphTopY,  YELLOW);
  }
  M5.Lcd.fillRect(graphRiseX1,graphTopY-10, 320, 5,  BLACK);
  M5.Lcd.fillRect(graphRiseX1,graphTopY-10, graphFlashPx, 5,  OLIVE);
  M5.Lcd.fillRect(graphRiseX1,graphBaseY+8, 320, 5,  BLACK);
  M5.Lcd.fillRect(graphRiseX1,graphBaseY+8, graphIntervalPx, 5,  PURPLE);
}

void drawFlashtimeFrame(){
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(flashTitleX, flashTitleY);
  M5.Lcd.println("Flashspeed");

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(flashUnitX, flashUnitY);
  M5.Lcd.println("us");
}

void drawFlashtime(){
  M5.Lcd.fillRect(flashX, flashY-5, flashX2-flashX, flashY2-flashY+10,  OLIVE);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(3);
  w = M5.Lcd.textWidth(strFlashtime);
  M5.Lcd.setCursor(flashX2-w, flashY);
  M5.Lcd.println(strFlashtime);
}

void drawFooter(){
  M5.Lcd.fillRoundRect( 10,220,  80,20,   5,   GREEN);
  M5.Lcd.fillRoundRect(230,220,  80,20,   5,   RED);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setCursor(45,224);
  M5.Lcd.println("+");
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setCursor(265,224);
  M5.Lcd.println("-");

  int x, y;
  if(mode == MODE_SET_FREQ){
    M5.Lcd.fillRoundRect(100,215, 120,30,  10,   DARKGREY);
    M5.Lcd.fillTriangle(160, 220, 155, 225, 165, 225,  CYAN);
    M5.Lcd.fillTriangle(160, 235, 155, 230, 165, 230, LIGHTGREY);
    x = freqX -10;
    y = (freqY2 + freqY) / 2;
    M5.Lcd.fillTriangle(x, y, x+10, y-10, x+10, y+10, GREEN);
    x = freqX2 + 10;
    M5.Lcd.fillTriangle(x, y, x-10, y-10, x-10, y+10, RED);
    x = flashX -15;
    y = (flashY2 + flashY) / 2;
    M5.Lcd.fillTriangle(x, y, x+10, y-10, x+10, y+10, BLACK);
    x = flashX2 + 15;
    M5.Lcd.fillTriangle(x, y, x-10, y-10, x-10, y+10, BLACK);
  }else if(mode == MODE_SET_FLASHTIME){
    M5.Lcd.fillRoundRect(100,215, 120,30,  10,   DARKGREY);
    M5.Lcd.fillTriangle(160, 220, 155, 225, 165, 225, LIGHTGREY);
    M5.Lcd.fillTriangle(160, 235, 155, 230, 165, 230, YELLOW);
    x = freqX -10;
    y = (freqY2 + freqY) / 2;
    M5.Lcd.fillTriangle(x, y, x+10, y-10, x+10, y+10, BLACK);
    x = freqX2 + 10;
    M5.Lcd.fillTriangle(x, y, x-10, y-10, x-10, y+10, BLACK);
    x = flashX -15;
    y = (flashY2 + flashY) / 2;
    M5.Lcd.fillTriangle(x, y, x+10, y-10, x+10, y+10, GREEN);
    x = flashX2 + 15;
    M5.Lcd.fillTriangle(x, y, x-10, y-10, x-10, y+10, RED);
  }
}

void switch_mode(){
  Serial.println("### swtich_mode()");
  if(mode == MODE_SET_FREQ){
    mode = MODE_SET_FLASHTIME;
    drawFlashtimeFrame();
    drawFlashtime();
  }else if(mode == MODE_SET_FLASHTIME){
    mode = MODE_SET_FREQ;
    Serial.println("---> MODE_SET_FREQ");
    drawFreqFrame();
    drawFreq();
  }
  drawFooter();
}

void updateFreqValues(){
  strFreq = floatToString(freq);

  interval = freqToInterval(freq);
  strInterval = uint64ToString(interval);
  Serial.println(strInterval);

  rpm = freq * 60;
  strRpm = uint64ToString(floatToUint64(rpm));
}

void updateFlashValues(){
  strFlashtime = String(flashtime);
  Serial.println(strFlashtime);
}

/* After M5Core is started or reset
  the program in the setUp () function will be run, and this part will only be run once.*/
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  M5.begin();  //Init M5Core.
  M5.Power.begin(); //Init Power module. 

  drawFreqFrame();
  drawFreq();
  drawGraphFrame();
  drawGraph();
  drawFlashtimeFrame();
  drawFlashtime();

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
 
    // Serial.print("Interrupt: ");
    // Serial.println(totalInterruptintervaler);
 
  }
  
  M5.update(); //Read the press state of the key.  
  
  switch(mode){
    
  case MODE_SET_FLASHTIME:
    if (M5.BtnB.wasReleased()){
      switch_mode();
    }else if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 10)) {
      flashtime++;
      if(flashtime >= interval){
        flashtime = interval;
      }
      updateFlashValues();
      updateTimer();
      drawFlashtime();
      drawGraph();
    } else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 10)) {
      flashtime--;
      if(flashtime <= minFlashtime){
        flashtime = minFlashtime;
      }
      updateFlashValues();
      updateTimer();
      drawFlashtime();
      drawGraph();
    }
    break;

  case MODE_SET_FREQ:
    if (M5.BtnB.wasReleased()){
      switch_mode();
    }else if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 10)) {
      if (freq > 100) {
        freq += 1;
      }else{
        freq += 0.1;
      }
      if(freq >= maxFreq){
        freq = maxFreq;
      }
      updateFreqValues();
      updateTimer();
      drawFreq();
      drawGraph();
    } else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 10)) {
      if (freq > 100){
        freq -= 1;
      }else{
        freq -= 0.1;
      }
      if(freq <= minFreq){
        freq = minFreq;
      }
      updateFreqValues();
      updateTimer();
      drawFreq();
      drawGraph();
    }
    break;
  }
}
