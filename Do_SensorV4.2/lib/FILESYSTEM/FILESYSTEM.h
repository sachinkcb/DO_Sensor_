/* 
  FileSystem.h -esp32 library that used portion of Internal Flash to backup
  frames(Data) in files.

  This lib used SPIFFS as file System, A portion of memory is used as Backup
  Storage.with Specific Numbers of files, The storage is used as circular 
  queue of files.

  Dev: Infiplus Team 
  Jan 2021  
  */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "FS.h"

#define MAXFILES 30
#define FS_NOT_MOUNTED (0)
#define FILE_OPEN_FAILED (-1)
#define FILE_WRITE_FAILED (-2)
#define FILE_WRITE_SUCCESSFUL (1)

class FILESYSTEM
{
private:
  /* data */
  bool fsMounted;

public:
  FILESYSTEM(/* args */);
  ~FILESYSTEM();
  bool begin();
  int writeFile(const char *path, const char *message);
  int readFile(const char *path, char *data);
  int getFileSize(const char *fname);
  bool isMounted(void);
  File openFile(const char *fname); 

  //protected:
};

#endif