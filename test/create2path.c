#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

  char * pathname = "Directory1";

  initializeSuperBlock();
  printAllEntries(openDirectory);
  //Cria primeiro diretorio
  LGA_LOGGER_TEST("Creating Directory1 on root directory");
  if(mkdir2(pathname) != 0)
  {
      LGA_LOGGER_TEST("mkdir FAILED");
      return -1;
  }
  LGA_LOGGER_TEST("mkdir SUCCEEDED");
  printAllEntries(openDirectory);

  LGA_LOGGER_TEST("Creating file on Directory1");
  if(create2("Directory1/MeuArquivoViaPath") < 0){
    LGA_LOGGER_TEST("1 creating FAILED");
    return -1;
  }
  printAllEntries(openDirectory);
  LGA_LOGGER_TEST("create2 SUCCEEDED");

  //Muda diretorio
  LGA_LOGGER_TEST("Changing to /Directory1 ");
  if(chdir2("/Directory1") != 0)
  {
      LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
      printAllEntries(openDirectory);
      return FAILED;
  }
  LGA_LOGGER_TEST("chdir to /Directory1 SUCCEEDED");
  printAllEntries(openDirectory);

  // /printAllEntries(openDirectory);

  }
