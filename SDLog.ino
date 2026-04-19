#include "SDLog.h"

SDLog::SDLog() {
  lineIndex = 0;
  fileIndex = 0;
  opened = false; 
}

void SDLog::log(struct RecordData *d) {
  if (lineIndex >= IN_FILE_LINES_LIMIT) {

    mF.close();
    opened = false;
    fileIndex++;
    lineIndex = 0;
  } else {
    if (lineIndex == 0 && !opened) {
      byte i = sizeof(newFolderNumber);
      while ( i-- ) *( fileName + i ) = *( newFolderNumber + i );
      strcat(fileName, "/");
      itoa(fileIndex,fileBuf,10);
      strcat(fileName,fileBuf);
      strcat(fileName,".log");
      mF = SD.open(fileName, FILE_WRITE);
      #ifdef DEBUG_SDLOG  
        Serial.print(F("File "));
        Serial.println(fileName);
      #endif
      opened = true;
    }

    add(d);
  
    if (d->index == 0)
      writeLine(d);
  }
}

void SDLog::add(struct RecordData *d) {
  byte *currentByteData, *lastFreePositon; 
  currentByteData = (byte *)&d->timer;
  lastFreePositon = (byte *)&d->bs[0] + d->index;
  if (d->index < ONE_LINE_BYTES_LIMIT) {
    for (int i = 0; i < ONE_RECORD_BYTES_SIZE; i++)
      *lastFreePositon++ = *currentByteData++;
    d->index += ONE_RECORD_BYTES_SIZE;
  } else { 
    d->bs[0] = '\0';
    d->index = 0;
  }
}

void SDLog::writeLine(struct RecordData *d) {
  mF.write((const uint8_t *)d->bs, ONE_LINE_BYTES_LIMIT);
  lineIndex++;
}

void SDLog::createNewFolder() {
  mF = SD.open("/");
  int lastfolderNum = 0;
  while (true) {
    File entry =  mF.openNextFile();
    if (! entry) {
      break;
    }
    if (entry.isDirectory() && checkTheFolderIsDigit(entry.name())) {
        lastfolderNum = max(lastfolderNum, atoi(entry.name()));
    }
    entry.close();
  }
  mF.close();
  itoa(lastfolderNum + 1, newFolderNumber, 10);
  if (SD.exists(newFolderNumber)) {
    #ifdef DEBUG_SDLOG
      Serial.println("Already exists!");
    #endif
    SD.remove(newFolderNumber);
  }
  SD.mkdir(newFolderNumber);
  mF = SD.open(newFolderNumber);
  #ifdef DEBUG_SDLOG
    Serial.print(F("Create folder ")); 
    Serial.print(newFolderNumber);
    Serial.print(F(": "));
    Serial.println(mF && mF.isDirectory()?F("successful"):F("failed")); 
    mF.close(); 
  #endif
  
}

boolean SDLog::checkTheFolderIsDigit(char * dirPointer) {
      while (*dirPointer) {
        if (!isDigit(dirPointer[0]))
          return false;
        dirPointer++;
      }
      return true;

}
