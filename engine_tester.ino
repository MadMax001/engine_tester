#include <AnalogKey.h>
#include <GyverButton.h>
#include <GyverHX711.h>

#include "SDLog.h"
//#define DEBUG_System

#define LOG_CHIP_SELECT_PIN 10         //chip_select для контроллера SD
#define BTN_RESET_PIN 2               //сброс (тарирование + запись в новый файл)
#define LED_PIN 8                     //пин индикаторного светодиода

#define LED_WAIT_DURATION 500         //полупериод мигания до начала измерения: 0.5 с горит, 0.5 с потушен
#define LED_ERROR_DURATION 125        //полупериод мигания при ошибке: 0.25 с горит, 0.25 с потушен

GyverHX711 sensor(7, 6, HX_GAIN64_A);
// HX_GAIN128_A - канал А усиление 128
// HX_GAIN32_B - канал B усиление 32
// HX_GAIN64_A - канал А усиление 64
SDLog sdlog;
GButton *butReset;
unsigned long startTime;
char buf[12];
struct RecordData logData;
boolean error;
boolean blink;                                     //текущее состояние индикатора
uint32_t cycleTimer;                               //таймер смены состояния индикации
enum Mode {WAIT_MODE, LOG_MODE} mode;              //WAIT_MODE - ожидание запуска, LOG_MODE - запись измерений
enum LED_MODE {LED_WAIT, LED_WORK, LED_ERR} ledStage;   //режим индикации: LED_WAIT - ожидание, LED_WORK - запись, LED_ERR - ошибка

void setup() {
  #ifdef DEBUG_System
    Serial.begin(9600);
  #endif
  error = false;
  blink = true;
  mode = WAIT_MODE;
  ledStage = LED_WAIT;
  cycleTimer = millis();
  hardSetup();
  if (error)
    Serial.print(F("Ошибка: ")); 
}

void hardSetup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(LOG_CHIP_SELECT_PIN, OUTPUT);
  butReset = new GButton(BTN_RESET_PIN);
  butReset->setTickMode(AUTO);
  butReset->resetStates();
  #ifdef DEBUG_System  
    Serial.print(F("Initializing SD card... "));
  #endif
  if (!SD.begin(LOG_CHIP_SELECT_PIN)) {
    #ifdef DEBUG_System  
      Serial.println(F("initialization failed!"));
    #endif
    error = true;
    return;
  } else {
    #ifdef DEBUG_System  
      Serial.println(F("initialization done."));
    #endif
  }
}

void loop() {
    if (sensor.available()) {
      unsigned long elapsed = millis() - startTime;
      long weight100 = (long)((int64_t)sensor.read() * 1000L / 524L);
      if (mode == LOG_MODE)
        logMeasuredData(elapsed, weight100);
      #ifdef DEBUG_System
        printDebugLine(elapsed, weight100);   
        if (Serial.available() > 0) {
          char c = Serial.read();
          if (c == ' ') {
            reset();
          }
        }
      #endif  
    }
    if (butReset->isRelease()) {
      reset();  
    }
//    butReset->tick();
    ledUpdate();
}

void reset() {
  #ifdef DEBUG_System
    Serial.println(F(">>> Выполняется тарирование (обнуление)..."));
  #endif
  startTime = millis();
  sensor.tare();
  if (!sdlog.createNewFolder()) {
    error = true;
    mode = WAIT_MODE;
    #ifdef DEBUG_System
      Serial.print(F("Ошибка SD: запуск записи не выполнен, код "));
      Serial.println(sdlog.getLastError());
    #endif
  } else {
    error = false;
    mode = LOG_MODE;
    #ifdef DEBUG_System
      Serial.println(F(">>> Запись измерений начата."));
    #endif
  }
}

void led() {
  digitalWrite(LED_PIN, blink ? HIGH : LOW);
}

void ledUpdate() {
  LED_MODE stage;
  if (error || sdlog.hasError())
    stage = LED_ERR;
  else if (mode == LOG_MODE)
    stage = LED_WORK;
  else
    stage = LED_WAIT;

  if (stage != ledStage) {             //смена режима индикации - сброс таймера стадии
    ledStage = stage;
    cycleTimer = millis();
    blink = true;
  }

  if (stage == LED_WAIT && millis() - cycleTimer >= LED_WAIT_DURATION) {
    blink = !blink;
    cycleTimer = millis();
  }
  if (stage == LED_ERR && millis() - cycleTimer >= LED_ERROR_DURATION) {
    blink = !blink;
    cycleTimer = millis();
  }
  if (stage == LED_WORK)
    blink = true;

  led();
}

void printDebugLine(unsigned long timer, long weight100) {
    Serial.print(F("  "));
    Serial.println(weight100 / 100.0);
}

void reportError(const __FlashStringHelper *action) {
    if (error) return;
    error = true;
    #ifdef DEBUG_System
      Serial.print(F("Ошибка SD: "));
      Serial.print(action);
      Serial.print(F(", код "));
      Serial.println(sdlog.getLastError());
    #endif
}

void logMeasuredData(unsigned long timer, long weight100) {
    logData.timer = timer;
    logData.weight100 = weight100;
    if (!sdlog.log(&logData))
      reportError(F("запись измерения не выполнена"));
}