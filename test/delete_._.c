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
  if(mkdir2(pathname) != SUCCEEDED)
  {
      LGA_LOGGER_TEST("mkdir FAILED");
      return -1;
  }
  LGA_LOGGER_TEST("mkdir SUCCEEDED");
  printAllEntries(openDirectory);


  //Muda para Directory1
  LGA_LOGGER_TEST("Changing to /Directory1 ");
  if(chdir2("/Directory1") != SUCCEEDED)
  {
      LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
      printAllEntries(openDirectory);
      return FAILED;
  }
  LGA_LOGGER_TEST("chdir to /Directory1 SUCCEEDED");
  create2("oi.txt");
  mkdir2("DirToDelete");
  printAllEntries(openDirectory);

  rmdir2(".");
  rmdir2("oi.txt");
  rmdir2("DirToDelete");
  printAllEntries(openDirectory);

  printf("Reseting:\n" );
  mkdir2("DirToDelete");
  printAllEntries(openDirectory);

  delete2(".");
  delete2("oi.txt");
  delete2("DirToDelete");
  printAllEntries(openDirectory);





  }
