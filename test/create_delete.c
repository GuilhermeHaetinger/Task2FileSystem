#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

    char * filename = "Arquivo1";
    char * filename2 = "Arquivo2";
    char * filename3 = "MesmoInodeArquivo2";

    LGA_LOGGER_TEST("Creating file on directory");
    if(create2(filename) < 0){
      LGA_LOGGER_TEST("1 creating FAILED");
      return -1;
    }
    printAllEntries(openDirectory);
    LGA_LOGGER_TEST("creating SUCCEEDED");

    LGA_LOGGER_TEST("Creating another file on new directory");
    if(create2(filename2) < 0){
      LGA_LOGGER_TEST("2 creating FAILED");
      return -1;
    }
    printAllEntries(openDirectory);
    LGA_LOGGER_TEST("creating SUCCEEDED");

    LGA_LOGGER_TEST("Deleting first file created on directory");
    if(delete2(filename) != SUCCEEDED){
      LGA_LOGGER_TEST("1 deleting FAILED");
      return -1;
    }
    printAllEntries(openDirectory);
    LGA_LOGGER_TEST("deleting SUCCEEDED");

    LGA_LOGGER_TEST("Creating file using first file created free inode and same name");
    if(create2(filename) < 0){
      LGA_LOGGER_TEST("3 creating FAILED");
      return -1;
    }
    printAllEntries(openDirectory);
    LGA_LOGGER_TEST("creating SUCCEEDED");

    LGA_LOGGER_TEST("Deleting second file created on directory");
    if(delete2(filename2) != SUCCEEDED){
      LGA_LOGGER_TEST("2 deleting FAILED");
      return -1;
    }
    printAllEntries(openDirectory);
    LGA_LOGGER_TEST("deleting SUCCEEDED");

    LGA_LOGGER_TEST("Creating file using second file created free inode");
    if(create2(filename3) < 0){
      LGA_LOGGER_TEST("4 creating FAILED");
      return -1;
    }
    printAllEntries(openDirectory);
    LGA_LOGGER_TEST("creating SUCCEEDED");

    return 0;
}
