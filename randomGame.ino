#define blinks 5

int timers[] = {100, 800};

#define butt_sens 0

#define debug 0

#define buzzPin 7
#define LEDGND 5
#define buzzGND 4
#define sensGND 3
#define sensVCC 6

byte timers_num = sizeof(timers) / 2;
byte LED[] = {11, 10, 9};
#include <LowPower.h>
byte fade_count;
volatile byte mode;
boolean cap_flag;
volatile boolean debonce_flag, threshold_flag;
volatile unsigned long debounce_time;
unsigned long last_fade, last_try;

byte count, try_count;
int wait_time[max_blinks], min_wait[max_blinks], max_wait[max_blinks];

void setup() {
  Serial.begin(9600);
  if (butt_sens) pinMode(2, INPUT_PULLUP);
  else pinMode(2, INPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(LEDGND, OUTPUT);
  pinMode(buzzGND, OUTPUT);
  pinMode(sensGND, OUTPUT);
  pinMode(sensVCC, OUTPUT);

  digitalWrite(buzzPin, 0);
  digitalWrite(LEDGND, 0);
  digitalWrite(buzzGND, 0);
  digitalWrite(sensGND, 0);
  digitalWrite(sensVCC, 1);

  for (int i = 0; i < 3; i++) {
    pinMode(LED[i], OUTPUT);
    digitalWrite(LED[i], 0);
  }

  if (butt_sens) attachInterrupt(0, threshold, FALLING);
  else attachInterrupt(0, threshold, RISING);

  good_night();
}

void fade (int color, int bright) {
  for (byte i = 0; i < 252; i += 2) {
    analogWrite(LED[color], i);
    delay(1);
  }
  tone(buzzPin, 500);
  delay(bright);
  noTone(buzzPin);
  for (byte i = 254; i > 3; i -= 2) {
    analogWrite(LED[color], i);
    delay(1);
  }
  digitalWrite(LED[color], 0);
}

void loop() {
  if (threshold_flag && mode == 0) {
    fade(1, 50);
    threshold_flag = 0;
    mode = 1;
    delay(800);
  }

  if (mode == 1) {                                
    randomSeed(millis());      
    for (byte i = 0; i < blinks; i++) {            
      byte rnd = random(0, timers_num);           
      wait_time[i] = timers[rnd];                 
      min_wait[i] = wait_time[i];    
      max_wait[i] = wait_time[i];
    }

    for (byte i = 0; i < blinks; i++) {
      fade(2, 30);
      delay(wait_time[i]);               
      if (debug) Serial.println(wait_time[i]);
    }
    mode = 2;
  }

  if (mode == 2) {
    threshold_flag = 0;
    debounce_time = millis();
    while (!threshold_flag) if (millis() - debounce_time > 5000) {
        fade(0, 300);
        delay(100);
        fade(0, 300);
        debounce_time = millis();
        mode = 0;
        if (debug) Serial.println("sucker");
        break;
      }
    if (threshold_flag) {
      fade(2, 30);
      last_try = millis();      
      try_count = 0;
      threshold_flag = 0;
      while (1) {

        if (millis() - last_try > max_wait[try_count]) {
          fade(0, 300);
          delay(1000);
          mode = 0;            
          if (debug) Serial.println("too slow");
          threshold_flag = 0;
          break;
        }
        if (threshold_flag) {

          if (millis() - last_try > min_wait[try_count] && millis() - last_try < max_wait[try_count]) {
            fade(2, 30);             
            try_count++;              
            last_try = millis();       
            threshold_flag = 0;        
            if (debug) Serial.println("good");

          } else if (millis() - last_try < min_wait[try_count] && threshold_flag) {
            fade(0, 300);
            delay(100);
            fade(0, 300);        
            delay(1000);
            mode = 0;        
            if (debug) Serial.println("too fast");
            threshold_flag = 0;
            break;
          }

          if (try_count == count - 1) {
            delay(200);
            fade(1, 100);
            delay(200);
            fade(1, 100);
            delay(200);
            fade(1, 100);
            delay(200);
            mode = 0;  
            if (debug) Serial.println("victory");
            break;
          }
        }
      }
    }
  }

  if (millis() - debounce_time > 10000) {
    good_night();
  }
}

void threshold() {
  if (millis() - debounce_time > 50) debonce_flag = 1;
  if (debonce_flag) {
    debounce_time = millis();
    threshold_flag = 1;
    debonce_flag = 0;
  }
}

void good_night() {
  if (debug) Serial.println("good night");
  delay(5);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}