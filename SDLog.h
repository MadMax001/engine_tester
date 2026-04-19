#ifndef SDLog_H
#define SDLog_H

#define DEBUG_SDLOG

#define ONE_LINE_BYTES_LIMIT 247              //количество байт в одной строке
#define ONE_RECORD_BYTES_SIZE 13              //размер одной структуры в байтах
#define IN_FILE_LINES_LIMIT 10                //количство строк в файле

#include <SPI.h>
#include <SD.h>

struct RecordData {
  long timer;       
  float weight;        
  byte bs[ONE_LINE_BYTES_LIMIT];    //байтовое представление
  int index;          
};

class SDLog {
  private:
    void add(struct RecordData *d);
    void writeLine(struct RecordData *d);
    void removeFiles(File mF, char fileName[]);
    boolean checkTheFolderIsDigit(char * dirPointer);
    
    File mF;
    char fileName[32];
    char fileBuf[32];
    int fileIndex;
    int lineIndex;
    char newFolderNumber[8];
    boolean opened;
  public:
    SDLog();
    void log(struct RecordData *d);
    void createNewFolder();
};

#endif 
