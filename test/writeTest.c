#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

    char * textToWrite = "My name is Guilherme Haetinger ";
    char * textToWrite2 = "and I want to be the best sisopper.";


    if(create2("guigosSecret.txt") != SUCCEEDED){
        LGA_LOGGER_TEST("Creation failed");
        return FAILED;
    }
    FILE2 openFile = open2("guigosSecret.txt");
    if(openFile < SUCCEEDED){
        LGA_LOGGER_TEST("Open 1 failed");
        return FAILED;
    }
    if(write2(openFile, textToWrite, strlen(textToWrite)) < SUCCEEDED){
        LGA_LOGGER_TEST("Write 1 failed");
        return FAILED;
    }
    FILE2 openFile2 = open2("guigosSecret.txt");
    if(openFile2 < SUCCEEDED){
        LGA_LOGGER_TEST("Open 2 failed");
        return FAILED;
    }

    if(seek2(openFile2, strlen(textToWrite)) != SUCCEEDED){
        LGA_LOGGER_TEST("Seek failed");
        return FAILED;
    }
    if(write2(openFile2, textToWrite2, strlen(textToWrite2)) < SUCCEEDED){
        LGA_LOGGER_TEST("write 2 failed");
        return FAILED;
    }
    FILE2 openFile3 = open2("guigosSecret.txt");
    if(openFile3 < SUCCEEDED){
        LGA_LOGGER_TEST("opening 3 failed");
        return FAILED;
    }
    char buffer[strlen(textToWrite) + strlen(textToWrite2) + 2];
    int read = read2(openFile3, buffer, strlen(textToWrite) + strlen(textToWrite2) + 2);
    if(read < SUCCEEDED){
        LGA_LOGGER_TEST("reading 3 failed");
        return FAILED;
    }
    printf("%d : buffer size \n %d : read size\n", strlen(textToWrite) + strlen(textToWrite2) + 2, read);
    int reader;

    for(reader = 0; reader < read; reader++){
        printf("%c", buffer[reader]);
    }
    FILE2 openFile4 = open2("guigosSecret.txt");
    if(openFile4 < SUCCEEDED){
        LGA_LOGGER_TEST("opening 4 failed");
        return FAILED;
    }
     if(seek2(openFile4, strlen(textToWrite)) != SUCCEEDED){
        LGA_LOGGER_TEST("Seek 4 failed");
        return FAILED;
    }
     if(truncate2(openFile4) != SUCCEEDED){
        LGA_LOGGER_TEST("Seek 4 failed");
        return FAILED;
    } 
    FILE2 openFile5 = open2("guigosSecret.txt");
    if(openFile5 < SUCCEEDED){
        LGA_LOGGER_TEST("opening 5 failed");
        return FAILED;
    }
    char buffer2[strlen(textToWrite) + strlen(textToWrite2) + 2];
    read = read2(openFile5, buffer2, strlen(textToWrite) + strlen(textToWrite2));
    if(read < SUCCEEDED){
        LGA_LOGGER_TEST("reading 5 failed");
        return FAILED;
    }
    printf("%d : buffer size \n %d : read size\n", strlen(textToWrite) + strlen(textToWrite2), read);

    for(reader = 0; reader < read; reader++){
        printf("%c", buffer2[reader]);
    }

    return 0;
}
