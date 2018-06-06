#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/t2fs.h"
#include "../include/LGA_support.h"
#include "../include/LGA_logger.h"

int main(){
  char buffer[80];
  int handler;
  for (int i = 0; i < 900; i ++) {
    snprintf(buffer, 80, "create%d.txt",i);
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
}
