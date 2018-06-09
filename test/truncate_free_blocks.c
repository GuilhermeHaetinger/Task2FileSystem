#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

    initializeSuperBlock();
    int a;

    char textToWrite[2 * BLOCK_SIZE_BYTES +  (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES];
    int i;
    for(i = 0; i < 2 * BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES; i++){
        textToWrite[i] = 'G';
    }
    LGA_LOGGER_TEST("/ directory");
    printAllEntries(openDirectory);
    printBitmap(BLOCK_TYPE, 3000, 1 );

    LGA_LOGGER_TEST("Prestes a criar arquivo");
    scanf(" %d\n", &a );
    create2("indTest.txt");
    FILE2 openFile2 = open2("indTest.txt");
    printBitmap(BLOCK_TYPE, 3000, 1 );
    LGA_LOGGER_TEST("after creating file");
    write2(openFile2, textToWrite, 2* BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES);
    openFile2 = open2("indTest.txt");

    char buffer[2 * BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES];
    int read = read2(openFile2, buffer, 2); //2 /** BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES*/
    if( read < SUCCEEDED){
        LGA_LOGGER_TEST("read failed");
        return FAILED;
    }

    scanf(" %d\n", &a );
    printBitmap(BLOCK_TYPE, 3000, 1 );
    LGA_LOGGER_TEST("after filling file");
    for(i = 0; i < 2 /** BLOCK_SIZE_BYTES + (BLOCK_SIZE_BYTES / sizeof(DWORD)) * BLOCK_SIZE_BYTES*/; i++){
        printf("%c", buffer[i]);
    }
    printf("\n" );

    scanf(" %d\n", &a );
    printf("truncate = %d \n", truncate2(openFile2));
    printBitmap(BLOCK_TYPE, 3000, 1 );
    LGA_LOGGER_TEST("after truncating file");



    return 0;
}
