/**
* Modification of duino https://github.com/ecto/duino
* to support 16x2 Serial LCD Display.
*/
#include <Servo.h>
#include <SoftwareSerial.h>
#include <serLCD.h>

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
Servo servo;
// lcd
int lcdPin = 4;
serLCD lcd = serLCD(lcdPin);
int divisor = 60;

void setup() {
  Serial.begin(115200);
  lcd.clear();
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
//  blinkDisplay();
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
  else if (strcmp(cmd, "97") == 0) {
    handleLCD(aux);
  } 
  else if (strcmp(cmd, "98") == 0) {
    handleServo(pin, val, aux);
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

/*
 * Handle Servo commands
 * attach, detach, write, read, writeMicroseconds, attached
 */
void handleServo(char *pin, char *val, char *aux) {
  if (debug) Serial.println("ss");
  int p = atoi(pin);
  Serial.println("got signal");
  if (strcmp(val, "00") == 0) {
    servo.detach();
  } 
  else if (strcmp(val, "01") == 0) {
    servo.attach(p);
    Serial.println("attached");
  } 
  else if (strcmp(val, "02") == 0) {
    Serial.println("writing to servo");
    Serial.println(atoi(aux));
    servo.write(atoi(aux));
  }
}

/*
* Handle Serial LCD commands
*/
void handleLCD(char *val) {
  if (debug) Serial.println(val);
  char *saved, *seconds;
  char *str;
  int index = 0;
  
  dw("13", "01");
  lcd.clear();
  for(str = val; ; str = NULL) {
    memset(seconds, 0, strlen(seconds));
    seconds = strtok_r(str, " ", &saved);
    if(seconds == NULL) {
      break;
    }
    seconds[strlen(seconds)] = '\0';
    if(index == 0) {
      lcd.selectLine(1);
      lcd.print(format(seconds));
    }
    else if(index == 1) {
      lcd.selectLine(2);
      lcd.print(format(seconds));
    }
    index = index + 1;
  }
  delay(1000);
  dw("13", "00");
}

String format(char *seconds) {
  int numSeconds = atoi(seconds);
  int dividend = numSeconds % divisor;
  int minutes;
  char time[16];
  if(numSeconds < 60) {
    sprintf(time, "%d seconds", numSeconds);
  }
  else {
    minutes = numSeconds / divisor;
    if(dividend == 0) {
      sprintf(time, "%d minutes", minutes);
    }
    else {
      sprintf(time, "%d min, %d secs", minutes, dividend);
    }
  }
  return time;
}
