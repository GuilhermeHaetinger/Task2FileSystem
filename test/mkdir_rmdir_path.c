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

  //Cria diretorio no primeiro diretorio criado
  LGA_LOGGER_TEST("Creating Directory2 on root Directory2 from root");
  if(mkdir2("./Directory1/Directory2") != SUCCEEDED)
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
  printAllEntries(openDirectory);

  //Cria diretorio no segundo diretorio com path absoluto
  LGA_LOGGER_TEST("Creating Directory3 on directory2 with absolut path");
  if(mkdir2("/Directory1/Directory2/Directory3") != SUCCEEDED)
  {
      LGA_LOGGER_TEST("mkdir FAILED");
      return -1;
  }
  LGA_LOGGER_TEST("mkdir SUCCEEDED");
  printAllEntries(openDirectory);

  //Muda para Directory2
  LGA_LOGGER_TEST("Changing to /Directory2 ");
  if(chdir2("./Directory2") != SUCCEEDED)
  {
      LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
      printAllEntries(openDirectory);
      return FAILED;
  }
  LGA_LOGGER_TEST("chdir to /Directory1 SUCCEEDED");
  printAllEntries(openDirectory);

  //Muda para /
  LGA_LOGGER_TEST("Changing to / ");
  if(chdir2("/") != SUCCEEDED)
  {
      LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
      printAllEntries(openDirectory);
      return FAILED;
  }
  LGA_LOGGER_TEST("chdir to / SUCCEEDED");
  printAllEntries(openDirectory);

  //Cria diretorio no segundo diretorio com path absoluto
  LGA_LOGGER_TEST("Removing Directory3 on directory2 on Directory1 with absolut path");
  if(rmdir2("/Directory1/Directory2/Directory3") != SUCCEEDED)
  {
      LGA_LOGGER_TEST("mkdir FAILED");
      return -1;
  }
  LGA_LOGGER_TEST("mkdir SUCCEEDED");
  printAllEntries(openDirectory);

  //Cria diretorio no segundo diretorio com path absoluto
  LGA_LOGGER_TEST("Creating Directory3 on directory2 with absolut path");
  if(mkdir2("/Directory1/Directory2") != SUCCEEDED)
  {
      LGA_LOGGER_TEST("mkdir FAILED");
      return -1;
  }
  LGA_LOGGER_TEST("mkdir SUCCEEDED");
  printAllEntries(openDirectory);


  }
