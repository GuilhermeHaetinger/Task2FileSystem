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

    //Cria primeiro diretorio
    LGA_LOGGER_TEST("Creating directory on root directory");
    if(mkdir2(pathname) != 0)
    {
        LGA_LOGGER_TEST("mkdir FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("mkdir SUCCEEDED");
    printAllEntries(openDirectory);

    //Muda diretorio
    LGA_LOGGER_TEST("Changing directory");
    if(chdir2("./Directory1") != 0)
    {
        LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
        printAllEntries(openDirectory);
        return FAILED;
    };
    LGA_LOGGER_TEST("chdir SUCCEEDED");
    printAllEntries(openDirectory);

    LGA_LOGGER_TEST("Creating file on directory");
    if(create2(filename) < 0){
      LGA_LOGGER_TEST("1 creating FAILED");
      return -1;
    }
    printAllEntries(openDirectory);
    LGA_LOGGER_TEST("create2 SUCCEEDED");

    //Cria segundo diretorio
    LGA_LOGGER_TEST("Creating directory on new directory");
    if(mkdir2(pathname2) != 0)
    {
        LGA_LOGGER_TEST("mkdir FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("mkdir SUCCEEDED");
    printAllEntries(openDirectory);

    //Muda diretorio
    LGA_LOGGER_TEST("Changing directory");
    if(chdir2("Directory2") != 0)
    {
        LGA_LOGGER_TEST("chdir FAILED - Returning to previous openDirectory");
        printAllEntries(openDirectory);
        return FAILED;
    }
    LGA_LOGGER_TEST("chdir SUCCEEDED");
    printAllEntries(openDirectory);
    return 0;
}
