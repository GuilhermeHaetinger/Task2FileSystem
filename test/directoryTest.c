#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/bitmap2.h"
#include "../include/LGA_support.h"
#include "../include/LGA_logger.h"

int main(){
  if (create2("create2teste.txt") < 0) {
    LGA_LOGGER_TEST("Create2 Failed");
  } else {
    LGA_LOGGER_TEST("Create2 SUCCEEDED and create2teste.txt should be printed");
    printAllEntries(openDirectory);
  }
}
