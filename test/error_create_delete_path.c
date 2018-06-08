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

  //Cria arquivo no diretorio
  LGA_LOGGER_TEST("Creating file on Directory2");
  if(create2("Directory2/MeuArquivoADeletarViaPath") != SUCCEEDED){
    LGA_LOGGER_TEST("Couldn't create on unexisting Directory2 ");
  }
  else
  {
    LGA_LOGGER_TEST("Something went wrong");
    return -1;
  }
  LGA_LOGGER_TEST("create2 forbidden SUCCEEDED");
  printAllEntries(openDirectory);
  //Muda para Directory1/Directory2
  LGA_LOGGER_TEST("Changing to /Directory1/Directory2 ");
  if(chdir2("/Directory1/Directory2") != 0)
  {
      LGA_LOGGER_TEST("Couldn't create on unexisting /Directory1/Directory2 ");

  }
  else
  {
    LGA_LOGGER_TEST("Something went wrong");
    return -1;
  }
  LGA_LOGGER_TEST("chdir forbidden to unexisting /Directory1/Directory2 SUCCEEDED");
  printAllEntries(openDirectory);

  //Muda para Directory1/Directory2
  LGA_LOGGER_TEST("Changing to /Directory1 ");
  if(chdir2("/Directory1") != SUCCEEDED)
  {
      LGA_LOGGER_TEST("Couldn't create on unexisting /Directory1/Directory2 ");

  }
  LGA_LOGGER_TEST("chdir to /Directory1  SUCCEEDED");
  printAllEntries(openDirectory);

  //Deleta arquivo no diretorio
  LGA_LOGGER_TEST("Deleting file on /Directory1");
  if(delete2("Directory1/MeuArquivoADeletarViaPath") != SUCCEEDED){
    LGA_LOGGER_TEST("Couldn't delete unexisting file in /Directory1 ");
  }
  else
  {
    LGA_LOGGER_TEST("Something went wrong");
    return -1;
  }



  }
