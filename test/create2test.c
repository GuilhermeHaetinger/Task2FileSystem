#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

  LGA_LOGGER_TEST("Creating file 'arquivoTeste'");

  FILE2 fileHandler = create2("arquivoTeste");

  if(fileHandler < 0){
      LGA_LOGGER_TEST("Create FAILED");
  } else {
    LGA_LOGGER_TEST("Create SUCCEDED, arquivoTeste should be printed");
    printAllEntries(openDirectory);
  }
  return 0;
}
