#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){
    char * filename = "filename";

    //Cria primeiro diretorio
    LGA_LOGGER_TEST("Creating Directory1 on root directory");
    if(mkdir2("Directory1") != SUCCEEDED)
    {
        LGA_LOGGER_TEST("mkdir FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("mkdir SUCCEEDED");
    printAllEntries(openDirectory);

    //Cria segundo diretorio
    LGA_LOGGER_TEST("Creating Directory2 on Directory1 ");
    if(mkdir2("Directory1/Directory2") != SUCCEEDED)
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

    LGA_LOGGER_TEST("Creating first file with handle = 0 in /Directory1/Directory2");
    FILE2 handle1 = create2("/Directory1/Directory2/filename");
    if(handle1 != 0){
        if(handle1 < 0){
            LGA_LOGGER_TEST("Error creating file");
        }else{
            LGA_LOGGER_TEST("Handle not properly set");
        }
        return -1;
    }
    LGA_LOGGER_TEST("Creating first file SUCCEEDED");

    LGA_LOGGER_TEST("Creating second file with handle = 1 in ./Directory2");
    FILE2 handle2 = create2("./Directory2/filename2");
    if(handle2 != 1){
        if(handle2 < 0){
            LGA_LOGGER_TEST("Error creating file");
        }else{
            LGA_LOGGER_TEST("Handle not properly set");
        }
        return -1;
    }
    LGA_LOGGER_TEST("Creating second file SUCCEEDED");

    if(close2(handle1) != 0){
        LGA_LOGGER_TEST("Closing file FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("Closing file SUCCEEDED");

    LGA_LOGGER_TEST("Opening first file with handle = 0(FIRST HANDLE AVAILABLE SINCE FIRST FILE WAS CLOSED)");
    FILE2 handle3 = open2("/Directory1/Directory2/filename");
    if(handle3 != 0){
        if(handle3 < 0){
            LGA_LOGGER_TEST("Error opening file");
        }else{
            LGA_LOGGER_TEST("Handle not properly set");
        }
        return -1;
    }
    LGA_LOGGER_TEST("Opening first file SUCCEEDED");
    LGA_LOGGER_TEST("Testing if the file was opened correctly");
    if(strcmp(openFiles[handle3].file.name, filename) != 0){
        LGA_LOGGER_TEST("File opening FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("File opened CORRECTLY");

    LGA_LOGGER_TEST("Opening first file AGAIN with handle = 2 with ./Directory2/filename");
    FILE2 handle4 = open2("./Directory2/filename");
    if(handle4 != 0){
        if(handle4 < 0){
            LGA_LOGGER_TEST("Error opening file AGAIN");
        }else{
            LGA_LOGGER_TEST("Handle not properly set AGAIN");
        }
        return -1;
    }
    LGA_LOGGER_TEST("Opening first file  AGAIN SUCCEEDED");
    LGA_LOGGER_TEST("Testing if the file was opened correctly AGAIN");
    if(strcmp(openFiles[handle4].file.name, filename) != 0){
        LGA_LOGGER_TEST("File opening FAILED AGAIN");
        return -1;
    }
    LGA_LOGGER_TEST("File opened CORRECTLY AGAIN");


    return 0;
}
