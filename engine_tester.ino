#include <AnalogKey.h>
#include <GyverButton.h>
#include <GyverHX711.h>

#include "SDLog.h"
#define DEBUG_System

#define LOG_CHIP_SELECT_PIN 5         //chip_select для контроллера SD
#define BTN_RESET_PIN 8               //сброс (тарирование + запись в новый файл)

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
  pinMode(LOG_CHIP_SELECT_PIN, OUTPUT);
  butReset = new GButton(BTN_RESET_PIN);
  butReset->setTickMode(AUTO);
  butReset->resetStates();
  #ifdef DEBUG_System  
    Serial.print(F("Initializing SD card... "));
  #endif
  if (!SD.begin(SD_CHIP_SELECT_PIN)) {
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
      float weight = sensor.read() / 52.4;
      logMeasuredData(elapsed, weight);
      if (butReset->isRelease()) {
        reset();  
      }
      #ifdef DEBUG_System
        printDebugLine(elapsed, weight);   
        if (Serial.available() > 0) {
          char c = Serial.read();
          if (c == ' ') {
            reset();
          }
        }
      #endif  
    }
}

void reset() {
  #ifdef DEBUG_System
    Serial.println(F(">>> Выполняется тарирование (обнуление)..."));
  #endif
  startTime = millis();
  sdlog.createNewFolder();
  sensor.tare(); 
}

void printDebugLine(unsigned long timer, float weight) {
    int minutes    = timer / 60000;
    int seconds    = (timer % 60000) / 1000;
    int hundredths = (timer % 1000) / 10;
    
    sprintf(buf, "%02d:%02d.%02d\r", minutes, seconds, hundredths);
  
    Serial.print(buf);
    Serial.print(F("  "));
    Serial.println(weight);
}

void logMeasuredData(unsigned long timer, float weight) {
    logData.timer = timer;
    logData.weight = weight;
    sdlog.log(&logData);
}
