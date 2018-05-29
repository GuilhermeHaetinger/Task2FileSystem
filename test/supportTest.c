#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/bitmap2.h"
#include "../include/LGA_support.h"
#include "../include/LGA_logger.h"

int inodeWritingTest();
int blockWritingTest();

int main(){

    LGA_LOGGER_TEST("initializing system");
    initializeSuperBlock();

    LGA_LOGGER_TEST("Setting blockFileSize value to 3");

    inodeWritingTest();
    blockWritingTest();
}

int inodeWritingTest(){
    Inode inode;
    LGA_LOGGER_TEST("Setting blockFileSize value to 3");
    inode.blocksFileSize = 3;

	  DWORD inodePos = searchBitmap2(INODE_TYPE, 0);
    LGA_LOGGER_TEST("Saving INODE");
    if(saveInode(inodePos, (char *)&inode) != SUCCEEDED){
        LGA_LOGGER_TEST("Saving INODE failed");
        return FAILED;
    }
    LGA_LOGGER_TEST("Saving INODE succeded");

    char test[INODE_SIZE];

    LGA_LOGGER_TEST("REcovering INODE");
    if(getInode(inodePos, test) != SUCCEEDED){
        LGA_LOGGER_TEST("Recovering INODE failed");
        return FAILED;
    }
    LGA_LOGGER_TEST("Recovering INODE succeded");

    if(((Inode*)test)->blocksFileSize != 3){
        LGA_LOGGER_TEST("Inode test FAILED");
        return FAILED;
    }

    LGA_LOGGER_TEST("Inode test SUCCEDED");
    return SUCCEEDED;

}

int blockWritingTest(){
    int i;
    char testingText[superBlock.blockSize * SECTOR_SIZE];

    LGA_LOGGER_TEST("Setting testingText");
    strcpy(testingText, "Hello World");

    LGA_LOGGER_TEST("Getting first block's sector number");
    int firstBlock = (superBlock.freeBlocksBitmapSize + superBlock.freeInodeBitmapSize + superBlock.inodeAreaSize + 1);

    LGA_LOGGER_TEST("Writing Block");
    if(writeBlock(firstBlock, testingText, strlen(testingText)) != SUCCEEDED){
        LGA_LOGGER_TEST("Block writing failed");
        return FAILED;
    }
    LGA_LOGGER_TEST("Block writing succeded");

    char testingBlock[superBlock.blockSize * SECTOR_SIZE];
    cleanArray(testingBlock, superBlock.blockSize * SECTOR_SIZE);

    LGA_LOGGER_TEST("Reading block");
    if(readBlock(firstBlock, testingBlock, superBlock.blockSize * SECTOR_SIZE) != SUCCEEDED){
        LGA_LOGGER_TEST("Reading block failed");
        return FAILED;
    }
    LGA_LOGGER_TEST("Reading block succeded");

    for(i = 0; i < 11; i++){
        if(testingText[i] != testingBlock[i]){
            LGA_LOGGER_TEST("Block Writing test FAILED");
            return FAILED;
        }
    }
    LGA_LOGGER_TEST("Block Writing test SUCCEDED");
    return SUCCEEDED;
}
