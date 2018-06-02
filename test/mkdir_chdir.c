#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"

int main(){
    
    char * pathname = "NewDirectory";
    char * filename = "FileForNewDirectory";

    LGA_LOGGER_TEST("Creating file on root directory");
    if(create2(filename) < 0){
        LGA_LOGGER_TEST("creating FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("creating SUCCEEDED");

    LGA_LOGGER_TEST("Creating directory on root directory");
    if(mkdir2(pathname) != 0){
        LGA_LOGGER_TEST("mkdir FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("mkdir SUCCEEDED");
    

    LGA_LOGGER_TEST("Moving to new directory");
    if(chdir2(pathname) != 0){
        LGA_LOGGER_TEST("chdir FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("chdir SUCCEEDED");
    

    LGA_LOGGER_TEST("Creating file on new directory");
    if(create2(filename) < 0){
        LGA_LOGGER_TEST("creating FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("creating SUCCEEDED");


    return 0;
}
