#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"
#include "../include/apidisk.h"
#include "../include/t2fs.h"

int openFilesHandler = 0;
int openDirectoriesHandler = 0;


int initializeSuperBlock(){
  if(!superBlockRead){
    LGA_LOGGER_LOG("Superblock wasn't read yet");
    return readSuperblock();
  }
  LGA_LOGGER_LOG("Superblock already read");
  return SUCCEEDED;
}

int readSuperblock(){

  if(superBlockRead){
    LGA_LOGGER_LOG("Superblock already read");

    return ALREADY_INITIALIZED;
  }

  unsigned char buffer[SECTOR_SIZE];

  if(read_sector(0, buffer) == SUCCEEDED){
    LGA_LOGGER_LOG("Superblock retrieved correctly");

    superBlock = *((SuperBlock*)buffer);

    inodeSectorIndex = (superBlock.freeBlocksBitmapSize + superBlock.freeInodeBitmapSize + 1) * superBlock.blockSize;

    superBlockRead = true;

    LGA_LOGGER_LOG("Superblock written correctly");
    return SUCCEEDED;
  }else{
    LGA_LOGGER_ERROR("Superblock retrieved incorrectly");
    return FAILED;
  }
}

int writeBlock(int initialSector, char* data){
  int dataSize = sizeof(data);

  if(blockIsInAcceptableSize(data)){
    LGA_LOGGER_LOG("Block in acceptable size");
    int i, j;

    char sectorBuffer[SECTOR_SIZE];

    LGA_LOGGER_LOG("Entering writing loop");

    for(i = 0; i < superBlock.blockSize; i++){

      for(j = 0; j< SECTOR_SIZE; j++){
        sectorBuffer[j] = data[j + i * SECTOR_SIZE];
      }

      if(write_sector(initialSector + i, sectorBuffer)){

      }else{
        LGA_LOGGER_ERROR("Writing failed in writing loop");
        return FAILED;
      }
    }
  }else{
    return FAILED;
  }


  LGA_LOGGER_LOG("Block written successfully");
  return SUCCEEDED;

}

int readBlock(int initialSector, char* data){
  int dataSize = sizeof(data);

  if(blockIsInAcceptableSize(data)){
    LGA_LOGGER_LOG("Block in acceptable size");
    int i, j;
    int readResult = SUCCEEDED;

    LGA_LOGGER_LOG("Entering reading loop");

    char sectorBuffer[SECTOR_SIZE];

    for(i = 0; i < superBlock.blockSize; i++){
      if(readResult == SUCCEEDED){
        readResult = read_sector(initialSector + i, sectorBuffer);
      }else{
        LGA_LOGGER_ERROR("reading failed in writing loop");
        return FAILED;
      }

      for(j = 0; j < SECTOR_SIZE; j++){
        data[j + i * SECTOR_SIZE] = sectorBuffer[j];
      }

    }
  }else{
    return FAILED;
  }

  LGA_LOGGER_LOG("Block read successfully");
  return SUCCEEDED;

}

bool blockIsInAcceptableSize(char* data){
  int dataSize = sizeof(data);

  int dataSectors = dataSize/SECTOR_SIZE;

  if(dataSectors != superBlock.blockSize){
    LGA_LOGGER_ERROR("Block isn't of acceptable size");
    return false;
  }else{
    return true;
  }
}

int saveInode(DWORD inodePos, char* data){
  int inodesPerSector = SECTOR_SIZE/INODE_SIZE;

  int inodeSectorPos = floor(inodePos/inodesPerSector);

  char inodeSector[SECTOR_SIZE];

  LGA_LOGGER_LOG("reading inode sector");
  if(read_sector(inodeSectorIndex + inodeSectorPos, inodeSector) != 0){
    LGA_LOGGER_ERROR("inode sector not read properly");
    return FAILED;
  }
  LGA_LOGGER_LOG("inode sector read properly");

  int offset = inodePos - (inodesPerSector * inodeSectorPos);


  char sectorData[SECTOR_SIZE];

  int i, j;
  LGA_LOGGER_LOG("Entering Inode writing loop");
  for(i = 0; i < inodesPerSector; i++){
    for(j = 0; j < INODE_SIZE; j++){

      //Se entramos no inode na posição de offset, o escrevemos
      if(i == offset){
        sectorData[i * INODE_SIZE + j] = data[j];

      //Caso o contrário, escrevemos o que já havia em disco
      }else{
        sectorData[i * INODE_SIZE + j] = inodeSector[i * INODE_SIZE + j];

      }
    }
  }

  LGA_LOGGER_LOG("writing new inode block");
  write_sector(inodeSectorIndex + inodeSectorPos, sectorData);

  char bufferTest[INODE_SIZE];
  for(i=0; i<INODE_SIZE; i++){
    bufferTest[i] = sectorData[i + offset * INODE_SIZE];
  }

  return SUCCEEDED;

}

int getSavedInode(DWORD inodePos, char* data){
  int inodesPerSector = SECTOR_SIZE/INODE_SIZE;

  int inodeSectorPos = inodePos/inodesPerSector;

  char inodeSector[SECTOR_SIZE];

  LGA_LOGGER_LOG("reading inode sector");
  if(read_sector(inodeSectorIndex + inodeSectorPos, inodeSector) != 0){
    LGA_LOGGER_ERROR("inode sector not read properly");
    return FAILED;
  }
  LGA_LOGGER_LOG("inode sector read properly");

  int offset = inodePos - (inodesPerSector * inodeSectorPos);

  char inodeData[INODE_SIZE];

  int i;
  LGA_LOGGER_LOG("Entering Inode reading loop");
  for(i=0; i<INODE_SIZE; i++){
    data[i] = inodeSector[i + offset * INODE_SIZE];
  }


  return SUCCEEDED;

}

FILE2 addFileToOpenFiles(FileRecord file){
  if(openFilesHandler >= MAX_NUM_OF_OPEN_FILES){
    LGA_LOGGER_IMPORTANT("File won't be added to vector since it's full");
    return FAILED;
  }

  LGA_LOGGER_LOG("recordHandler being created");
  recordHandler record;
  record.file = file;
  record.CP = 0;

  LGA_LOGGER_LOG("recordHandler being added");
  openFiles[openFilesHandler] = record;

  LGA_LOGGER_LOG("handler being increased");
  openFilesHandler++;

  return openFilesHandler - 1;

}

int getFreeNode() {
  LGA_LOGGER_LOG("[getFreeNode] Getting Free iNode");

  return searchBitmap2(INODE_TYPE, INODE_FREE);
}
