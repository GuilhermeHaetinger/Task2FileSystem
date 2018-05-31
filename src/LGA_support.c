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

int openFilesHandler       = 0;
int openDirectoriesHandler = 0;
int INODE_SECTOR_INDEX     = 0;
int INODE_PER_SECTOR       = 0;
int SECTORS_PER_BLOCK      = 0;

int initializeSuperBlock(){
  if(!superBlockRead){
    LGA_LOGGER_DEBUG("Superblock wasn't read yet");

    if(readSuperblock() != SUCCEEDED){
      LGA_LOGGER_ERROR("[initializeSystem] superBlock not read properly");
      return FAILED;
    }
    if(getRootInode((char *)&openDirectory) != SUCCEEDED){
      LGA_LOGGER_ERROR("[initializeSystem] Root inode not retrieved correctly");
      return FAILED;
    }
    LGA_LOGGER_DEBUG("Root Inode retrieved correctly");
    return SUCCEEDED;
  }
  LGA_LOGGER_DEBUG("Superblock already read");
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

    SECTORS_PER_BLOCK = superBlock.blockSize;
    superBlockRead = true;

    LGA_LOGGER_LOG("Superblock written correctly");
    return SUCCEEDED;
  }else{
    LGA_LOGGER_ERROR("Superblock retrieved incorrectly");
    return FAILED;
  }
}

int getFreeBlock(){
  return searchBitmap2(BLOCK_TYPE, 0);
}

int writeBlock(int blockPos, char* data, int dataSize) {

  int sectorPos = blockPos * SECTORS_PER_BLOCK;

  char sectorBuffer[SECTOR_SIZE];
  int i,j;

  setBitmap2(BLOCK_TYPE, blockPos, 1);

  for(i = 0; i < SECTORS_PER_BLOCK; i++){
    cleanArray(sectorBuffer, SECTOR_SIZE);
    for(j = 0; j< SECTOR_SIZE; j++){
      if (dataSize < j + i * SECTOR_SIZE)
        break;
      sectorBuffer[j] = data[j + i * SECTOR_SIZE];
    }
    if(write_sector(sectorPos + i, sectorBuffer) != SUCCEEDED){
      LGA_LOGGER_ERROR("[writeBlock] Writing failed in writing loop");
      return FAILED;
    }
  }
  LGA_LOGGER_LOG("[writeBlock] Successfully");

  return SUCCEEDED;
}

int readBlock(int blockPos, char* data, int dataSize){

  int sectorPos = blockPos * SECTORS_PER_BLOCK;

  if (dataSize != SECTORS_PER_BLOCK * SECTOR_SIZE) {
    LGA_LOGGER_ERROR("[readBlock] Data size is different from Block size");
    return FAILED;
  }

  int i, j;

  LGA_LOGGER_DEBUG("[readBlock] Entering reading loop");

  char sectorBuffer[SECTOR_SIZE];

  for(i = 0; i < superBlock.blockSize; i++){
    if(read_sector(sectorPos + i, sectorBuffer) != SUCCEEDED){
      LGA_LOGGER_ERROR("[readBlock] reading failed in writing loop");
      return FAILED;
    }
    for(j = 0; j < SECTOR_SIZE; j++){
      data[j + i * SECTOR_SIZE] = sectorBuffer[j];
    }
  }
  LGA_LOGGER_LOG("[readBlock] Block read successfully");
  return SUCCEEDED;

}

int getDataBlockPosition(int dataBlockPos){
  return 1 + superBlock.freeBlocksBitmapSize + superBlock.freeInodeBitmapSize + superBlock.inodeAreaSize + dataBlockPos;
}

int saveInode(DWORD inodePos, char* data){

  LGA_LOGGER_DEBUG("[saveInode] Setting iNode to busy on bitmap");
  if(getBitmap2(INODE_TYPE, inodePos) == INODE_BUSY){
    LGA_LOGGER_ERROR("[saveInode] Inode Bitmap positionis already busy");
    return FAILED;
  }
  setBitmap2(INODE_TYPE, inodePos, INODE_BUSY);
  int inodeSectorPos = getSectorIndexInode(inodePos);

  char diskSector[SECTOR_SIZE];
  LGA_LOGGER_DEBUG("[saveInode] Reading inode sector");
  if(read_sector(inodeSectorPos, diskSector) != SUCCEEDED){
    LGA_LOGGER_ERROR("[saveInode] inode sector not read properly");
    return FAILED;
  }
  LGA_LOGGER_DEBUG("[saveInode] inode sector read properly");

  int offset = getOffsetInode(inodePos);

  char sectorData[SECTOR_SIZE];
  changeSector(offset, data, INODE_SIZE, diskSector, sectorData);
  LGA_LOGGER_DEBUG("[saveInode] writing new inode block");

  write_sector(inodeSectorPos, sectorData);
  LGA_LOGGER_LOG("[saveInode] Successfully");
  return SUCCEEDED;
}


int setInode(DWORD inodePos, char* data){

  LGA_LOGGER_DEBUG("[setInode] Setting iNode to busy on bitmap");
  setBitmap2(INODE_TYPE, inodePos, INODE_BUSY);
  int inodeSectorPos = getSectorIndexInode(inodePos);

  char diskSector[SECTOR_SIZE];
  LGA_LOGGER_DEBUG("[setInode] Reading inode sector");
  if(read_sector(inodeSectorPos, diskSector) != SUCCEEDED){
    LGA_LOGGER_ERROR("[setInode] inode sector not read properly");
    return FAILED;
  }
  LGA_LOGGER_DEBUG("[setInode] inode sector read properly");

  int offset = getOffsetInode(inodePos);

  char sectorData[SECTOR_SIZE];
  changeSector(offset, data, INODE_SIZE, diskSector, sectorData);
  LGA_LOGGER_DEBUG("[setInode] writing new inode block");

  write_sector(inodeSectorPos, sectorData);
  LGA_LOGGER_LOG("[setInode] Successfully");
  return SUCCEEDED;
}

int getInode(DWORD inodePos, char* data){

  int inodesPerSector = SECTOR_SIZE/INODE_SIZE;

  int inodeSectorPos = getSectorIndexInode(inodePos);

  char inodeSector[SECTOR_SIZE];
  LGA_LOGGER_LOG("reading inode sector");
  if(read_sector(inodeSectorPos, inodeSector) != 0){
    LGA_LOGGER_ERROR("inode sector not read properly");
    return FAILED;
  }
  LGA_LOGGER_LOG("inode sector read properly");

  int offset = getOffsetInode(inodePos);

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

int getFreeInode() {
  LGA_LOGGER_LOG("[getFreeNode] Getting Free iNode");

  return searchBitmap2(INODE_TYPE, INODE_FREE);
}

void initializeInode(Inode * buffer){
  buffer->blocksFileSize = 0;
  buffer->bytesFileSize  = 0;
  buffer->dataPtr[0]     = INVALID_PTR;
  buffer->dataPtr[1]     = INVALID_PTR;
  buffer->singleIndPtr   = INVALID_PTR;
  buffer->doubleIndPtr   = INVALID_PTR;       
}

int getSectorIndexInode(DWORD inodePos) {
  LGA_LOGGER_DEBUG("[getSectorInode] Getting");
  return floor(inodePos/INODE_PER_SECTOR) + INODE_SECTOR_INDEX;
}

int getOffsetInode(DWORD inodePos) {
  int inodeSectorPos = getSectorIndexInode(inodePos);
  return inodePos - (INODE_PER_SECTOR * (inodeSectorPos - INODE_SECTOR_INDEX));
}

void changeSector(int start, char* data, int dataSize, char* diskSector, char* saveSector) {
  int i, j;
  LGA_LOGGER_DEBUG("[changeSector] Changing the Sector");
  for(i = 0; i < INODE_PER_SECTOR; i++){
    for(j = 0; j < dataSize; j++){
      if(i == start){
        saveSector[i * INODE_SIZE + j] = data[j];
      }else{
        saveSector[i * INODE_SIZE + j] = diskSector[i * INODE_SIZE + j];
      }
    }
  }
}

void cleanArray(char *array, int size) {
  int i;

  for(i=0; i < size; i++) {
    array[i] = 0;
  }
}

int createRoot() {
  
  Inode rootInode;
  initializeInode(&rootInode);
  
  rootInode.blocksFileSize = 1;
  rootInode.bytesFileSize = REGISTER_SIZE * 2;

  FileRecord dot, dotdot; /* . && .. */
  dot.TypeVal = TYPEVAL_DIRETORIO;
  strcpy(dot.name, ".");
  dot.inodeNumber = ROOT_INODE;
  dotdot.TypeVal = TYPEVAL_DIRETORIO;
  strcpy(dotdot.name, "..");
  dotdot.inodeNumber = ROOT_INODE;

  int sectorPos = searchBitmap2(BITMAP_DADOS, 0);
  char registersData[REGISTER_SIZE * 2];

  concatCustom(registersData, 0, (char*)&dot, REGISTER_SIZE);
  concatCustom(registersData, REGISTER_SIZE, (char*)&dotdot, REGISTER_SIZE);
  writeBlock(sectorPos, registersData, REGISTER_SIZE * 2);

  rootInode.dataPtr[0] = sectorPos;

  saveInode(ROOT_INODE, (char *)&rootInode);
  LGA_LOGGER_LOG("[createRoot] Root created");
  return SUCCEEDED;
}

int rootCreated() {
  if (getBitmap2(INODE_TYPE, ROOT_INODE) == 0) {
    LGA_LOGGER_LOG("[createRoot] Root isnt there");
    return FAILED;
  }
  LGA_LOGGER_LOG("[createRoot] Root is already there");
  return SUCCEEDED;
}

int getRootInode(char* buffer) {
  
  if (rootCreated() != SUCCEEDED) {
    if (createRoot() != SUCCEEDED) {
      LGA_LOGGER_ERROR("[getRoot] There's no Root");
      return FAILED;
    }
  }

  if (getInode(ROOT_INODE, buffer) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getRoot] Root iNode couldnt be read");
    return FAILED;
  }

  LGA_LOGGER_LOG("[getRoot] Get Root Successfully");
  return SUCCEEDED;
}

void concatCustom(char* concatened, int concatStartPos, char* buffer, int bufferSize) {
  int i;
  for(i=0; i<bufferSize; i++) {
    concatened[concatStartPos + i] = buffer[i];
  }
}

int getRegisterFile(int sectorPos, int registerNumber, char *buffer) {
  char diskSector[SECTOR_SIZE];

  if (read_sector(sectorPos, diskSector) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getRegisterFile] Sector couldnt be read");
    return FAILED;
  }
  if (getDataSector(registerNumber * REGISTER_SIZE, REGISTER_SIZE, diskSector, buffer) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getRegisterFile] Sector couldnt get the data");
    return FAILED;
  }

  return SUCCEEDED;
}

int getDataSector(int start, int dataSize, char* diskSector, char* buffer) {
  if (dataSize > SECTOR_SIZE) {
    LGA_LOGGER_ERROR("[getDataSector] DATA size greater than SECTOR size");
    return FAILED;
  }

  for (int i=0; i < dataSize; i++) {
    buffer[i] = diskSector[start + i];
  }
  LGA_LOGGER_DEBUG("[getDataSector] Successfully");
  return SUCCEEDED;
}

int addFileToOpenDirectory(FileRecord file){
}

int getNewFilePositionOnOpenDirectory(){
  
  if(openDirectory.dataPtr[0] == INVALID_PTR){
    ///ALOCA BLOCO PARA DATAPTR[0] E DÁ POSIÇÃO 1
  }else{
    int firstDirectPtrSector = getDataBlockPosition(openDirectory.dataPtr[0]) * SECTORS_PER_BLOCK;
    char record_buffer[REGISTER_SIZE];
    int sector, fileRec;
    for(sector = 0; sector < SECTORS_PER_BLOCK; sector++){
      for(fileRec = 0; fileRec < REGISTERS_PER_SECTOR; fileRec++){

        getRegisterFile(firstDirectPtrSector + sector, fileRec, record_buffer);
        if((*((FileRecord*)record_buffer)).TypeVal != TYPEVAL_DIRETORIO && (*((FileRecord*)record_buffer)).TypeVal != TYPEVAL_REGULAR){
          break;
        }
      }
    }

  if(sector == SECTOR_SIZE){
    ///FAZ O MESMO PARA DATAPTR[1]
  }  

  ///FAZER PARA INDIRETO SIMPLES E DUPLO

  }
}

int allocateDataBlock(Inode inode){
  if(inode.dataPtr[0] == INVALID_PTR){
    inode.dataPtr[0] = getFreeBlock();
    if(inode.dataPtr[0] < 0){
      LGA_LOGGER_ERROR("[allocateDataBlock] No available blocks");
      return FAILED;
    }
    setBitmap2(BLOCK_TYPE, inode.dataPtr[0], 1);
    LGA_LOGGER_DEBUG("[allocateDataBlock] Block allocated properly");
    inode.blocksFileSize++;
    return SUCCEEDED;
  }
  if(inode.dataPtr[1] == INVALID_PTR){
    inode.dataPtr[1] = getFreeBlock();
    if(inode.dataPtr[1] < 0){
      LGA_LOGGER_ERROR("[allocateDataBlock] No available blocks");
      return FAILED;
    }
    setBitmap2(BLOCK_TYPE, inode.dataPtr[1], 1);
    LGA_LOGGER_DEBUG("[allocateDataBlock] Block allocated properly");
    inode.blocksFileSize++;
    return SUCCEEDED;
  }
  ///TODO IND E DOUB IND
}