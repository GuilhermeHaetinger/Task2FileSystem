#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

    char * pathname = "MeuDiretorio";
    char * filename = "Vamooo";


    //Cria primeiro diretorio
    LGA_LOGGER_TEST("Creating MeuDiretorio on root directory");
    if(mkdir2(pathname) != 0)
    {
        LGA_LOGGER_TEST("mkdir FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("mkdir SUCCEEDED");

    //Cria o arquivo via path
    LGA_LOGGER_TEST("Creating file on /MeuDiretorio/");
    if(create2("/MeuDiretorio/Vamoo.txt") < 0){
      LGA_LOGGER_TEST("1 creating FAILED");
      return -1;
    }
    LGA_LOGGER_TEST("create2 SUCCEEDED");

    //Muda diretorio para /MeuDiretorio
    LGA_LOGGER_TEST("Changing to /MeuDiretorio");
    if(chdir2("/MeuDiretorio") != 0)
    {
      LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
      printAllEntries(openDirectory);
      return FAILED;
    }
    LGA_LOGGER_TEST("chdir SUCCEEDED");
    /*printAllEntries(openDirectory);

    //Volta diretorio raiz
    LGA_LOGGER_TEST("Changing to /");
    if(chdir2("/") != 0)
    {
      LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
      printAllEntries(openDirectory);
      return FAILED;
    }
    LGA_LOGGER_TEST("chdir SUCCEEDED");

    //Deleta o arquivo via path
    LGA_LOGGER_TEST("Deleting file on /MeuDiretorio/");
    if(delete2("/MeuDiretorio/Vamoo.txt") < 0){
      LGA_LOGGER_TEST("1 creating FAILED");
      return -1;
    }
    LGA_LOGGER_TEST("delete2 SUCCEEDED");

    //Muda diretorio para /MeuDiretorio
    LGA_LOGGER_TEST("Changing to /MeuDiretorio");
    if(chdir2("/MeuDiretorio") != 0)
    {
      LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
      printAllEntries(openDirectory);
      return FAILED;
    }
    LGA_LOGGER_TEST("chdir SUCCEEDED");*/
    printAllEntries(openDirectory);


}
