/**
* Modification of duino https://github.com/ecto/duino
* to support 16x2 Serial LCD Display.
* 
* Shifter library // http://bildr.org/2011/08/74hc595-breakout-arduino/
*/
#include <SoftwareSerial.h>
#include <serLCD.h>
#include <Shifter.h>

// 16x2 LCD = 32
// + (cmd+pin+val) = 6
// + /0
// ---
// 39
char messageBuffer[39];
int index = 0;
int msglength = 0;
char cmd[3];
char pin[3];
char val[3];
// see above equation
char aux[33];
bool debug = false;

// lcd
int lcdPin = 4;
serLCD lcd = serLCD(lcdPin);
char *lastMessage;
int blinkCount;
int blinkLimit = 3;
// lcd msg formatting
int divisor = 60;
char *in = "In:";
char *out = "Out:";
String msg1 = "In: N/A";
String msg2 = "Out: N/A";

// Shift Register
int dataPin = 11;
int latchPin = 12;
int clockPin = 8;
Shifter shifter(11, 12, 8, 2); 

void setup() {
  Serial.begin(115200);
  lcd.clear();
  shifter.clear();
}

void loop() {
  while(Serial.available() > 0) {
    char x = Serial.read();
    if (x == '!') {
      index = 0;      // start
      msglength = 0;
      memset(aux, 0, strlen(aux));
      memset(messageBuffer, 0, 39);
    }
    else if (x == '.') {
      msglength = index;
      messageBuffer[index++] = '/0';
      process(); // end
    }
    else messageBuffer[index++] = x;
  }
  // blinkDisplay();
}

/*
 * Deal with a full message and determine function to call
 */
void process() {
  index = 0;

  strncpy(cmd, messageBuffer, 2);
  cmd[2] = '\0';
  strncpy(pin, messageBuffer + 2, 2);
  pin[2] = '\0';
  strncpy(val, messageBuffer + 4, 2);
  val[2] = '\0';
  strncpy(aux, messageBuffer + 6, msglength - 6);
  aux[strlen(aux)] = '\0';

  if (debug) {
    Serial.println(messageBuffer);
  }

  if (strcmp(cmd, "00") == 0) {
    sm(pin, val);
  } 
  else if (strcmp(cmd, "01") == 0) {
    dw(pin, val); // digital write
  } 
  else if (strcmp(cmd, "02") == 0) {
    dr(pin, val); //  digital read
  } 
  else if (strcmp(cmd, "96") == 0) {
    shiftPin(pin, val);
  }
  else if (strcmp(cmd, "97") == 0) {
    handleLCD(val, aux);
  }
  else if (strcmp(cmd, "99") == 0) {
    toggleDebug(val);
  }
}

/*
 * Toggle debug mode
 */
void toggleDebug(char *val) {
  if (strcmp(val, "00") == 0) {
    debug = false;
    Serial.println("goodbye");
  } 
  else {
    debug = true;
    Serial.println("hello");
  }
}

/*
 * Set pin mode
 */
void sm(char *pin, char *val) {
  if (debug) Serial.println("sm");
  int p = atoi(pin);
  if (strcmp(val, "00") == 0) {
    pinMode(p, OUTPUT);
  } 
  else {
    pinMode(p, INPUT);
  }
}

/*
 * Digital write
 */
void dw(char *pin, char *val) {
  if (debug) Serial.println("dw");
  int p = atoi(pin);
  pinMode(p, OUTPUT);
  if (strcmp(val, "00") == 0) {
    digitalWrite(p, LOW);
  } 
  else {
    digitalWrite(p, HIGH);
  }
}

/*
 * Digital read
 */
void dr(char *pin, char *val) {
  if (debug) Serial.println("dr");
  int p = atoi(pin);
  pinMode(p, INPUT);
  int oraw = digitalRead(p);
  char m[4];
  char out[4];
  sprintf(m, "%d", p);
  sprintf(out, "%d", oraw);
  strcat(m, "::");
  strcat(m, out);
  Serial.println(m);
}

void shiftPin(char *pin, char *val) {
  int p = atoi(pin);
   shifter.setPin(p, (strcmp(val, "00") == 0) ? LOW : HIGH);
   shifter.write();
}

/*
* Handle Serial LCD commands
*/
void handleLCD(char *val, char *aux) {
  if (debug) Serial.println(val);
  blinkCount = 0;
  int line = (strcmp(val, "00") == 0) ? 0 : 1;
  lcd.clear();
  if(line == 0) {
    msg1 = format((line==0) ? in : out, aux);
  }
  else {
    msg2 = format((line==0) ? in : out, aux);
  }
  lcd.selectLine(1);
  lcd.print(msg1);
  lcd.selectLine(2);
  lcd.print(msg2);
  delay(100);
}

String format(char *dir, char *seconds) {
  int numSeconds = atoi(seconds);
  int dividend = numSeconds % divisor;
  int minutes;
  char time[16];
  if(numSeconds < 60) {
    sprintf(time, "%s %d seconds", dir, numSeconds);
  }
  else {
    minutes = numSeconds / divisor;
    if(dividend == 0) {
      sprintf(time, "%s %d minutes", dir, minutes);
    }
    else {
      sprintf(time, "%s %d m, %d s", dir, minutes, dividend);
    }
  }
  return time;
}

/*
void blinkDisplay() {
  // blink
  if(strlen(lastMessage) > 0 && (blinkCount++ < blinkLimit)) {
    lcd.clear();
    delay(1000);
    lcd.print(lastMessage);
    delay(1000);
    lcd.clear();
  }
  // scroll
  else if(strlen(lastMessage) > 0) {
    removeFirst(lastMessage);
    lcd.clear();
    lcd.print(lastMessage);
    delay(500);
  }
}

void removeFirst (char *s) {
    if (*s == '\0') return;
    *s = *(s+1);
    removeFirst (s+1);
}
*/

