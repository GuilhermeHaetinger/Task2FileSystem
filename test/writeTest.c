#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

    char * writingText = "Writing";
    int size = 7;
    int i;

    FILE2 handle = create2("FileTest.txt");

    Inode fileInode;
    getInode(openFiles[handle].file.inodeNumber, (char *)&fileInode);

    char dataprt1test[superBlock.blockSize * SECTOR_SIZE];

    for(i = 0; i < superBlock.blockSize * SECTOR_SIZE; i++){
        dataprt1test[i] = 'G';
    }

    writeOnFile(&fileInode, openFiles[handle].CP, writingText, size);
    writeOnFile(&fileInode,  7, dataprt1test, superBlock.blockSize * SECTOR_SIZE);

    char writing[superBlock.blockSize * SECTOR_SIZE + 7];
    
    fileInode.bytesFileSize = superBlock.blockSize * SECTOR_SIZE + 7;
    
    readFileBlocks(fileInode, 0, writing, superBlock.blockSize * SECTOR_SIZE + 7);


    for(i = 0; i < superBlock.blockSize * SECTOR_SIZE + 9; i++){
        printf("%c", writing[i]);
    }


    return 0;
}
