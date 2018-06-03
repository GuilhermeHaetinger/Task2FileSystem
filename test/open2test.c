#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"

int main(){
    char * filename = "filename";
    char * filename2 = "filename2";

    LGA_LOGGER_TEST("Creating first file with handle = 0");
    FILE2 handle1 = create2(filename);
    if(handle1 != 0){
        if(handle1 < 0){
            LGA_LOGGER_TEST("Error creating file");
        }else{
            LGA_LOGGER_TEST("Handle not properly set");
        }
        return -1;    
    }
    LGA_LOGGER_TEST("Creating first file SUCCEEDED");

    LGA_LOGGER_TEST("Creating second file with handle = 1");
    FILE2 handle2 = create2(filename2);
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
    FILE2 handle3 = open2(filename);
    if(handle3 != 0){
        if(handle3 < 0){
            LGA_LOGGER_TEST("Error creating file");
        }else{
            LGA_LOGGER_TEST("Handle not properly set");
        }
        return -1;    
    }
    LGA_LOGGER_TEST("Opening first file SUCCEEDED");

    return 0;
}
