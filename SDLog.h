#ifndef SDLog_H
#define SDLog_H

#define DEBUG_SDLOG

#define ONE_LINE_BYTES_LIMIT 240              //количество байт в одной строке
#define ONE_RECORD_BYTES_SIZE 8               //размер одной записи в байтах (long timer + long weight100)
#define IN_FILE_LINES_LIMIT 10                //количство строк в файле
#define RECORDS_PER_LINE (ONE_LINE_BYTES_LIMIT / ONE_RECORD_BYTES_SIZE)   //количество записей в одной строке

#define SDLOG_OK                 0            //ошибки нет
#define SDLOG_ERR_OPEN_ROOT      1            //не удалось открыть корень SD-карты
#define SDLOG_ERR_CREATE_FOLDER  2            //не удалось создать каталог
#define SDLOG_ERR_OPEN_FOLDER    3            //не удалось открыть созданный каталог
#define SDLOG_ERR_OPEN_FILE      4            //не удалось открыть файл для записи
#define SDLOG_ERR_WRITE          5            //ошибка записи данных в файл

#include <SPI.h>
#include <SD.h>

struct RecordData {
  long timer;
  long weight100;        //вес, умноженный на 100 (сохраняем 2 знака после запятой)
};

class SDLog {
  private:
    void add(struct RecordData *d);
    boolean writeLine();
    void removeFiles(File mF, char fileName[]);
    boolean checkTheFolderIsDigit(char * dirPointer);
    
    File mF;
    char fileName[32];
    char fileBuf[32];
    int fileIndex;
    int lineIndex;
    byte recInLine;
    byte lineBuf[ONE_LINE_BYTES_LIMIT];     //буфер текущей строки (RECORDS_PER_LINE записей)
    char newFolderNumber[8];
    boolean opened;
    byte errorCode;
  public:
    SDLog();
    boolean log(struct RecordData *d);      //true - запись выполнена успешно
    boolean createNewFolder();              //true - каталог создан успешно
    boolean hasError();                     //статус ошибки: true - ошибка есть
    byte getLastError();                    //код последней ошибки (SDLOG_*)
};

#endif