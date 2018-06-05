#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

    char * pathname = "Directory1";
    char * pathname2 = "Directory2";
    char * filename = "Arquivo1";
    char * filename2 = "Vamooo";


    //Cria primeiro diretorio
    LGA_LOGGER_TEST("Creating Directory1 on root directory");
    if(mkdir2(pathname) != 0)
    {
        LGA_LOGGER_TEST("mkdir FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("mkdir SUCCEEDED");
    printAllEntries(openDirectory);

    //Muda diretorio
    LGA_LOGGER_TEST("Changing to /Directory1 ");
    if(chdir2("/Directory1") != 0)
    {
        LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
        printAllEntries(openDirectory);
        return FAILED;
    };
    LGA_LOGGER_TEST("chdir to /Directory1 SUCCEEDED");
    printAllEntries(openDirectory);

    LGA_LOGGER_TEST("Creating file on Directory1");
    if(create2(filename) < 0){
      LGA_LOGGER_TEST("1 creating FAILED");
      return -1;
    }
    printAllEntries(openDirectory);
    LGA_LOGGER_TEST("create2 SUCCEEDED");

    //Cria segundo diretorio
    LGA_LOGGER_TEST("Creating Directory2 on Directory1");
    if(mkdir2(pathname2) != 0)
    {
        LGA_LOGGER_TEST("mkdir FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("mkdir SUCCEEDED");
    printAllEntries(openDirectory);

    //Muda diretorio
    LGA_LOGGER_TEST("Changing to /Directory1/Directory2");
    if(chdir2("/Directory1/Directory2") != 0)
    {
        LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
        printAllEntries(openDirectory);
        return FAILED;
    }
    LGA_LOGGER_TEST("chdir SUCCEEDED");
    printAllEntries(openDirectory);

    //Criar arquivo no segundo diretorio criado
    LGA_LOGGER_TEST("Creating file on Directory2");
    if(create2(filename2) < 0){
      LGA_LOGGER_TEST("1 creating FAILED");
      return -1;
    }
    printAllEntries(openDirectory);
    LGA_LOGGER_TEST("create2 SUCCEEDED");

    //Muda pro diretorio raiz
    LGA_LOGGER_TEST("Changing to root /");
    if(chdir2("/") != 0)
    {
      LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
      printAllEntries(openDirectory);
      return FAILED;
    }
    LGA_LOGGER_TEST("chdir SUCCEEDED");
    printAllEntries(openDirectory);

    //Muda diretorio acima - é pra ser o raiz
    LGA_LOGGER_TEST("Changing /Directory1/Directory2");
    if(chdir2("Directory1/Directory2") != 0)
    {
      LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
      printAllEntries(openDirectory);
      return FAILED;
    }
    LGA_LOGGER_TEST("chdir SUCCEEDED");
    printAllEntries(openDirectory);

}
