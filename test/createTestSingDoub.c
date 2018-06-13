#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/t2fs.h"
#include "../include/LGA_support.h"
#include "../include/LGA_logger.h"

int main(){
  char buffer[80];
  int handler;
  int i;
  for ( i = 0; i < 15000; i ++) {
    snprintf(buffer, 80, "queijos%d.txt",i);
    handler = create2(buffer);

    if (handler < 0) {
      LGA_LOGGER_ERROR("Create2 Failed");
      printAllEntries(openDirectory);
      return 1;
    } else {
      close2(handler);
      LGA_LOGGER_TEST("Create2 SUCCEEDED and create2teste.txt should be printed");
    }
  }

  printAllEntries(openDirectory);
  char block[1024];
  char ptr[sizeof(DWORD)];
  readBlock(openDirectory.doubleIndPtr, block, 1024);
  printBlock(openDirectory.doubleIndPtr);
  puts("====");
  getDataFromDisk(ptr, sizeof(DWORD) * 1, sizeof(DWORD), block, 1024);
  printBlock(*(DWORD*)ptr);
  readBlock(*(DWORD*)ptr, block, 1024);
  getDataFromDisk(ptr, 0, sizeof(DWORD), block, 1024);
  puts("====");
  printBlock(*(DWORD*)ptr);
}
