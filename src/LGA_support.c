#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"
#include "../include/apidisk.h"
#include "../include/t2fs.h"



int checkAndReadSuperBlock(){
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

    strncpy(superBlock.id, ((SuperBlock*)buffer)->id, 4);
    superBlock.version              = ((SuperBlock*)buffer)->version;
    superBlock.superblockSize       = ((SuperBlock*)buffer)->superblockSize;
    superBlock.freeBlocksBitmapSize = ((SuperBlock*)buffer)->freeBlocksBitmapSize; 
    superBlock.freeInodeBitmapSize  = ((SuperBlock*)buffer)->freeInodeBitmapSize;
    superBlock.inodeAreaSize        = ((SuperBlock*)buffer)->inodeAreaSize;
    superBlock.blockSize            = ((SuperBlock*)buffer)->blockSize;
    superBlock.diskSize             = ((SuperBlock*)buffer)->diskSize;

    inodeSectorIndex = (superBlock.freeBlocksBitmapSize + superBlock.freeInodeBitmapSize) * superBlock.blockSize + 1;

    superBlockRead = true;

    return SUCCEEDED;
  }else{
    LGA_LOGGER_ERROR("Superblock retrieved incorrectly");
    return FAILED;
  }
}

int writeBlock(int initialSector, char* data){
  int dataSize = sizeof(data);

  if(blockIsInAcceptableSize(data)){
    int i;
    int dataSectors = ceil(dataSize/SECTOR_SIZE);
    int writeResult = SUCCEEDED;

    LGA_LOGGER_LOG("Entering writing loop");

    for(i = 0; i < dataSectors; i++){
      if(writeResult == SUCCEEDED){
        writeResult = write_sector(initialSector + i, data + i * SECTOR_SIZE);
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

bool blockIsInAcceptableSize(char* data){
  int dataSize = sizeof(data);

  int dataSectors = dataSize/SECTOR_SIZE;

  if(dataSectors > superBlock.blockSize){
    LGA_LOGGER_LOG("Block isn't of acceptable size");
    return false;
  }else{
    return true;
  }
}

int saveInode(DWORD inodePos, char* data){
  int inodesPerSector = SECTOR_SIZE/INODE_SIZE;

  int inodeSectorPos = inodePos/inodesPerSector;

  char inodeSector[SECTOR_SIZE];
  
  if(read_sector(inodeSectorIndex + inodeSectorPos, inodeSector) != 0){
    LGA_LOGGER_ERROR("inode sector not read properly");
    return FAILED;
  }

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

  writeBlock(inodeSectorIndex + inodeSectorPos, sectorData);

  return SUCCEEDED;

}

int getSavedInode(DWORD inodePos, char* data){
  int inodesPerSector = SECTOR_SIZE/INODE_SIZE;

  int inodeSectorPos = inodePos/inodesPerSector;

  char inodeSector[SECTOR_SIZE];
  
  if(read_sector(inodeSectorIndex + inodeSectorPos, inodeSector) != 0){
    LGA_LOGGER_ERROR("inode sector not read properly");
    return FAILED;
  }

  int offset = inodePos - (inodesPerSector * inodeSectorPos);

  char inodeData[INODE_SIZE];

  int i, j;
  LGA_LOGGER_LOG("Entering Inode writing loop");
  for(i = 0; i < inodesPerSector; i++){
    for(j = 0; j < INODE_SIZE; j++){

      //Se entramos no inode na posição de offset, o escrevemos
      if(i == offset){
        inodeData[j] = inodeSector[i * INODE_SIZE + j];
      }
    }
  }

  return SUCCEEDED;

}


int addFileToOpenFiles(FileRecord file){
  if(openFilesHandler >= MAX_NUM_OF_OPEN_FILES){
    LGA_LOGGER_IMPORTANT("File won't be added to vector since it's full");
    return FAILED;
  }

  openFiles[openFilesHandler] = file;
  openFilesHandler++;

  return openFilesHandler;

}