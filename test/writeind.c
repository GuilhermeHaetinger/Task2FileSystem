#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

    initializeSuperBlock();


    char textToWrite[2 * BLOCK_SIZE_BYTES +  (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES];
    int i;
    for(i = 0; i < 2 * BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES; i++){
        textToWrite[i] = 'G';
    }
   /* printf("to write : %d", 2 * BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES);
    for(i = 0; i < 2 * BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES; i++){
        printf("%c", textToWrite[i]);
    }*/
    if(create2("indTest.txt") != SUCCEEDED){
        LGA_LOGGER_TEST("Creation failed");
        return FAILED;
    }
    FILE2 openFile = open2("indTest.txt");
    if(openFile < SUCCEEDED){
        LGA_LOGGER_TEST("Open 1 failed");
        return FAILED;
    }
    if(write2(openFile, textToWrite, 2 * BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES) < SUCCEEDED){
        LGA_LOGGER_TEST("Write 1 failed");
        return FAILED;
    }
    FILE2 openFile2 = open2("indTest.txt");
    if(openFile2 < SUCCEEDED){
        LGA_LOGGER_TEST("Open 2 failed");
        return FAILED;
    }
    char buffer[2 * BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES];
    int read = read2(openFile2, buffer, 2 * BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES);
    if( read < SUCCEEDED){
        LGA_LOGGER_TEST("read failed");
        return FAILED;
    }

    for(i = 0; i < 2 * BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES; i++){
        printf("%c", buffer[i]);
    }

    return 0;
}
