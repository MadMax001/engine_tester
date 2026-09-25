#include "SDLog.h"

SDLog::SDLog() {
  lineIndex = 0;
  recInLine = 0;
  fileIndex = 0;
  opened = false; 
  errorCode = SDLOG_OK;
}

boolean SDLog::openFile() {
  byte i = sizeof(newFolderNumber);
  while ( i-- ) *( fileName + i ) = *( newFolderNumber + i );
  strcat(fileName, "/");
  itoa(fileIndex, fileBuf, 10);
  strcat(fileName, fileBuf);
  strcat(fileName, ".log");
  mF = SD.open(fileName, FILE_WRITE);
  if (!mF) {
    errorCode = SDLOG_ERR_OPEN_FILE;
    #ifdef DEBUG_SDLOG
      Serial.print(F("File open failed: "));
      Serial.println(fileName);
    #endif
    return false;
  }
  opened = true;
  lineIndex = 0;
  recInLine = 0;
  #ifdef DEBUG_SDLOG
    Serial.print(F("File "));
    Serial.println(fileName);
  #endif
  return true;
}

boolean SDLog::log(struct RecordData *d) {
  if (errorCode != SDLOG_OK) return false;
  if (!opened) {
    if (!openFile()) return false;
  }

  add(d);

  if (recInLine >= RECORDS_PER_LINE)
    if (!writeLine()) return false;

  return true;
}

void SDLog::add(struct RecordData *d) {
  memcpy(lineBuf + recInLine * ONE_RECORD_BYTES_SIZE, d, ONE_RECORD_BYTES_SIZE);
  recInLine++;
}

boolean SDLog::writeLine() {
  if (mF.write((const uint8_t *)lineBuf, ONE_LINE_BYTES_LIMIT) != ONE_LINE_BYTES_LIMIT) {
    errorCode = SDLOG_ERR_WRITE;
    mF.close();
    opened = false;
    recInLine = 0;
    return false;
  }
  mF.flush();
  recInLine = 0;
  lineIndex++;
  if (lineIndex >= IN_FILE_LINES_LIMIT) {
    mF.close();
    opened = false;
    fileIndex++;
    if (!openFile()) return false;
  }
  return true;
}

boolean SDLog::createNewFolder() {
  errorCode = SDLOG_OK;
  if (opened) {
    if (recInLine > 0) {
      int tailBytes = recInLine * ONE_RECORD_BYTES_SIZE;
      if (mF.write((const uint8_t *)lineBuf, tailBytes) != tailBytes) {
        errorCode = SDLOG_ERR_WRITE;
        mF.close();
        opened = false;
        recInLine = 0;
        lineIndex = 0;
        return false;
      }
      mF.flush();
    }
    mF.close();
    opened = false;
    recInLine = 0;
    lineIndex = 0;
  }
  File root = SD.open("/");
  if (!root) {
    errorCode = SDLOG_ERR_OPEN_ROOT;
    return false;
  }
  int lastfolderNum = 0;
  while (true) {
    File entry =  root.openNextFile();
    if (! entry) {
      break;
    }
    if (entry.isDirectory() && checkTheFolderIsDigit(entry.name())) {
        lastfolderNum = max(lastfolderNum, atoi(entry.name()));
    }
    entry.close();
  }
  root.close();
  itoa(lastfolderNum + 1, newFolderNumber, 10);
  if (SD.exists(newFolderNumber)) {
    #ifdef DEBUG_SDLOG
      Serial.println("Already exists. Remove!");
    #endif
    SD.remove(newFolderNumber);
  }
  #ifdef DEBUG_SDLOG
    Serial.print(F("Try create folder "));
    Serial.println(newFolderNumber);
  #endif
  bool result = SD.mkdir(newFolderNumber);
  Serial.print("Создан каталог: ");
  Serial.println(result?F("successful"):F("failed"));
  if (!result) {
    errorCode = SDLOG_ERR_CREATE_FOLDER;
    return false;
  }
  #ifdef DEBUG_SDLOG
    Serial.print(F("Create folder "));
    Serial.print(newFolderNumber);
    Serial.println(SD.exists(newFolderNumber) ? F(": successful") : F(": failed"));
  #endif
  fileIndex = 1;
  if (!openFile()) return false;
  return true;
}

boolean SDLog::checkTheFolderIsDigit(char * dirPointer) {
      while (*dirPointer) {
        if (!isDigit(dirPointer[0]))
          return false;
        dirPointer++;
      }
      return true;

}

boolean SDLog::hasError() {
  return errorCode != SDLOG_OK;
}

byte SDLog::getLastError() {
  return errorCode;
}