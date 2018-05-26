#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/bitmap2.h"

int openFilesHandler = 0;
int openDirectoriesHandler = 0;
int INODE_SECTOR_INDEX;
int INODE_PER_SECTOR;

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

    INODE_SECTOR_INDEX = (superBlock.freeBlocksBitmapSize + superBlock.freeInodeBitmapSize + 1) * superBlock.blockSize;

    INODE_PER_SECTOR = SECTOR_SIZE/INODE_SIZE;
    superBlockRead = true;

    LGA_LOGGER_LOG("Superblock written correctly");
    return SUCCEEDED;
  }else{
    LGA_LOGGER_ERROR("Superblock retrieved incorrectly");
    return FAILED;
  }
}

int writeBlock(int initialSector, char* data){
  if (initializeSuperBlock() == FAILED) {
    LGA_LOGGER_ERROR("SuperBlock wasn't initialized");
    return FAILED;
  }
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
  if (initializeSuperBlock() == FAILED) {
    LGA_LOGGER_ERROR("SuperBlock wasn't initialized");
    return FAILED;
  }
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
  if (initializeSuperBlock() == FAILED) {
    LGA_LOGGER_ERROR("SuperBlock wasn't initialized");
    return FAILED;
  }

  int inodeSectorPos = getSectorIndexInode(inodePos);
  printf("%d\n",inodeSectorPos );

  char diskSector[SECTOR_SIZE];
  LGA_LOGGER_LOG("[saveInode] Reading inode sector");
  if(read_sector(inodeSectorPos, diskSector) != SUCCEEDED){
    LGA_LOGGER_ERROR("[saveInode] inode sector not read properly");
    return FAILED;
  }
  LGA_LOGGER_LOG("[saveInode] inode sector read properly");

  int offset = getOffsetInode(inodePos);
  printf("%d\n",offset );

  char sectorData[SECTOR_SIZE];
  changeSector(offset, data, diskSector, sectorData);
  LGA_LOGGER_LOG("[saveInode] writing new inode block");
  write_sector(inodeSectorPos, sectorData);

  return SUCCEEDED;
}

int getSavedInode(DWORD inodePos, char* data){
  if (initializeSuperBlock() == FAILED) {
    LGA_LOGGER_ERROR("SuperBlock wasn't initialized");
    return FAILED;
  }
  
  int inodesPerSector = SECTOR_SIZE/INODE_SIZE;

  int inodeSectorPos = getSectorIndexInode(inodePos);

  char inodeSector[SECTOR_SIZE];
  printf("%d\n",inodeSectorPos );
  LGA_LOGGER_LOG("reading inode sector");
  if(read_sector(inodeSectorPos, inodeSector) != 0){
    LGA_LOGGER_ERROR("inode sector not read properly");
    return FAILED;
  }
  LGA_LOGGER_LOG("inode sector read properly");

  int offset = getOffsetInode(inodePos);
  printf("%d\n",offset );

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

int getSectorIndexInode(DWORD inodePos) {
  LGA_LOGGER_DEBUG("[getSectorInode]");
  return floor(inodePos/INODE_PER_SECTOR) + INODE_SECTOR_INDEX;
}

int getOffsetInode(DWORD inodePos) {
  int inodeSectorPos = getSectorIndexInode(inodePos);
  return inodePos - (INODE_PER_SECTOR * (inodeSectorPos - INODE_SECTOR_INDEX));
}

void changeSector(int start, char* data, char* diskSector, char* saveSector) {
  int size = sizeof(data);
  int i, j;

  LGA_LOGGER_DEBUG("[changeSector] Changing the Sector");
  for(i = 0; i < INODE_PER_SECTOR; i++){
    for(j = 0; j < size; j++){
      if(i == start){
        saveSector[i * INODE_SIZE + j] = data[j];
      }else{
        saveSector[i * INODE_SIZE + j] = diskSector[i * INODE_SIZE + j];
      }
    }
  }
}

void getDataSector(int start, int end, char* diskSector, char* saveSector) {

}
