#include <AnalogKey.h>
#include <GyverButton.h>
#include <GyverHX711.h>

#include "SDLog.h"
#define DEBUG_System

#define LOG_CHIP_SELECT_PIN 10         //chip_select для контроллера SD
#define BTN_RESET_PIN 2               //сброс (тарирование + запись в новый файл)

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

void setup() {
  #ifdef DEBUG_System
    Serial.begin(9600);
  #endif
  error = false;
  hardSetup();
  if (!error)
    reset();
  else
    Serial.print(F("Ошибка: ")); 
}

void hardSetup() {
  delay(1000);
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
      logMeasuredData(elapsed, weight100);
      if (butReset->isRelease()) {
        reset();  
      }
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
    butReset->tick();
}

void reset() {
  #ifdef DEBUG_System
    Serial.println(F(">>> Выполняется тарирование (обнуление)..."));
  #endif
  startTime = millis();
  if (!sdlog.createNewFolder()) {
    error = true;
    #ifdef DEBUG_System
      Serial.print(F("Ошибка SD: создание каталога не выполнено, код "));
      Serial.println(sdlog.getLastError());
    #endif
  } else if (error) {
    error = false;
    #ifdef DEBUG_System
      Serial.println(F(">>> Запись на SD восстановлена."));
    #endif
  }
  sensor.tare(); 
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
