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
int BLOCK_SIZE_BYTES       = 0;
int REGISTERS_PER_BLOCK    = 0;
int FIRST_ENTRY            = 0;
int SECOND_ENTRY           = 0;
int SINGLE_ENTRY           = 0;
int DOUBLE_ENTRY           = 0;

/* ################################ */
/* --------- SUPER_BLOCK_SECTION  ---------- */
/* ################################ */

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

int readSuperblock() {
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

    BLOCK_SIZE_BYTES = SECTORS_PER_BLOCK * SECTOR_SIZE;

    REGISTERS_PER_BLOCK = BLOCK_SIZE_BYTES / REGISTER_SIZE;

    FIRST_ENTRY = REGISTERS_PER_BLOCK - 1;
    SECOND_ENTRY = (REGISTERS_PER_BLOCK * 2) - 1;
    SINGLE_ENTRY = (REGISTERS_PER_BLOCK * REGISTERS_PER_BLOCK) - 1;
    DOUBLE_ENTRY = (REGISTERS_PER_BLOCK * REGISTERS_PER_BLOCK * REGISTERS_PER_BLOCK) - 1;
    superBlockRead = true;

    LGA_LOGGER_LOG("Superblock written correctly");
    return SUCCEEDED;
  }else{
    LGA_LOGGER_ERROR("Superblock retrieved incorrectly");
    return FAILED;
  }
}

/* ################################ */
/* --------- FILE_REGISTER_SECTION  ---------- */
/* ################################ */

int getRegisterFile(int registerNumber, char* diskBuffer, int diskBufferSize, char *buffer) {
  if (registerNumber * REGISTER_SIZE > diskBufferSize) {
    LGA_LOGGER_ERROR("[getRegisterFile] registerNumber is greater than diskBufferSize");
    return FAILED;
  }
  if (getDataFromDisk(buffer, registerNumber * REGISTER_SIZE, REGISTER_SIZE, diskBuffer, diskBufferSize) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getRegisterFile] Sector couldnt get the data");
    return FAILED;
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

int addFileToOpenDirectory(FileRecord file){
  int position = getNewFilePositionOnOpenDirectory();
  printf("%d\n", position);
  if (position < 0) {
    LGA_LOGGER_ERROR("[addFileToOpenDirectory] Couldnt add file to directory");
    return FAILED;
  }
  if (writeFilePositionInInode(openDirectory, (char*)&file, position) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[addFileToOpenDirectory]");
    return FAILED;
  }
  LGA_LOGGER_LOG("[addFileToOpenFiles] Success");
  return SUCCEEDED;
}

int getNewFilePositionOnOpenDirectory(){

  int try = searchNewFileRecordPosition(&(openDirectory.dataPtr[0]));

  if (try >= 0) {
    LGA_LOGGER_LOG("[getNewFilePositionOnOpenDirectory] Success");
    return try;
  } else if (try == FAILED) {
    LGA_LOGGER_ERROR("[getNewFilePositionOnOpenDirectory] Couldnt get the position");
    return FAILED;
  }

  try = searchNewFileRecordPosition(&(openDirectory.dataPtr[1]));
  if (try >= 0) {
    LGA_LOGGER_LOG("[getNewFilePositionOnOpenDirectory] Success");
    return try + (REGISTERS_PER_BLOCK * 1);
  } else if (try == FAILED) {
    LGA_LOGGER_ERROR("[getNewFilePositionOnOpenDirectory] Couldnt get the position");
    return FAILED;
  }
  LGA_LOGGER_ERROR("[getNewFilePositionOnOpenDirectory] Couldnt get the position");
  return FAILED;
}

int searchNewFileRecordPosition(DWORD *ptr) {
  if (*ptr == INVALID_PTR) {
    LGA_LOGGER_LOG("[searchNewFileRecordPosition] Invalid Ptr");
    *ptr = getFreeBlock();
    if(*ptr < 0){
      LGA_LOGGER_ERROR("[searchNewFileRecordPosition] No available blocks");
      return FAILED;
    }
  }
  char blockBuffer[BLOCK_SIZE_BYTES], registerBuffer[REGISTER_SIZE];

  readBlock(*ptr, blockBuffer, BLOCK_SIZE_BYTES);

  for (int position = 0; position < BLOCK_SIZE_BYTES/REGISTER_SIZE; position++) {
    if (getRegisterFile(position, blockBuffer, BLOCK_SIZE_BYTES, registerBuffer) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[searchNewFileRecordPosition] Couldnt get the register");
      return FAILED;
    }
    if(((FileRecord*)registerBuffer)->TypeVal == 0) {
      LGA_LOGGER_LOG("[searchNewFileRecordPosition] Get the position");
      return position;
    }
  }
  LGA_LOGGER_LOG("[searchNewFileRecordPosition] Couldnt get the position");
  return BLOCK_FULL;
}

int printAllEntries(Inode inode) {

  if (inode.dataPtr[0] != INVALID_PTR) {
    if (_printEntries(inode.dataPtr[0]) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[printAllEntries] couldnt print the first entry");
      return FAILED;
    }
  }
  if (inode.dataPtr[1] != INVALID_PTR) {
    if (_printEntries(inode.dataPtr[0]) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[printAllEntries] couldnt print the second entry");
      return FAILED;
    }
  }
  printf("\n");

  LGA_LOGGER_LOG("[printAllEntries] Success");
}

int writeFilePositionInInode(Inode inode, char *fileRecord, int position) {
  if (position < 0) {
    return FAILED;
  }
  if (position <= FIRST_ENTRY) {
    if (changeWriteBlock(inode.dataPtr[0], position * REGISTER_SIZE, fileRecord, REGISTER_SIZE) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[writeFilePositionInInode]");
      return FAILED;
    }
    LGA_LOGGER_LOG("[writeFilePositionInInode] Success");
    return SUCCEEDED;
  } else if (position <= SECOND_ENTRY) {

  } else if (position <= SINGLE_ENTRY) {

  } else if (position <= DOUBLE_ENTRY) {

  }
}

/* ################################ */
/* --------- BLOCK_SECTION ---------- */
/* ################################ */

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

int changeWriteBlock(int blockPos, int dataPos, char* data, int dataSize) {
  char diskBuffer[BLOCK_SIZE_BYTES], newDisk[BLOCK_SIZE_BYTES];
  if (readBlock(blockPos, diskBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[changeWriteBlock]");
    return FAILED;
  }

  changeDisk(dataPos, data, dataSize, diskBuffer, BLOCK_SIZE_BYTES, newDisk);

  if (writeBlock(blockPos, newDisk, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[changeWriteBlock]");
    return FAILED;
  }
  LGA_LOGGER_LOG("OPA");

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

/* ################################ */
/* ---------- INODE_SECTION ---------- */
/* ################################ */

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
  changeSectorInode(offset, data, INODE_SIZE, diskSector, sectorData);
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
  changeSectorInode(offset, data, INODE_SIZE, diskSector, sectorData);
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

void changeSectorInode(int start, char* data, int dataSize, char* diskSector, char* saveSector) {
  int i, j;
  LGA_LOGGER_DEBUG("[changeSectorInode] Changing the Sector");
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

/* ################################ */
/* ----------   ROOT_SECTION    ---------- */
/* ################################ */

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

  setInode(ROOT_INODE, (char *)&rootInode);
  LGA_LOGGER_LOG("[createRoot] Root created");
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

int rootCreated() {
  if (getBitmap2(INODE_TYPE, ROOT_INODE) == 0) {
    LGA_LOGGER_LOG("[createRoot] Root isnt there");
    return FAILED;
  }
  LGA_LOGGER_LOG("[createRoot] Root is already there");
  return SUCCEEDED;
}

/* ################################ */
/* ---------- AUXILIARES  --------- */
/* ################################ */

int getDataBlockPosition(int dataBlockPos){
  return 1 + superBlock.freeBlocksBitmapSize + superBlock.freeInodeBitmapSize + superBlock.inodeAreaSize + dataBlockPos;
}

int getSectorIndexInode(DWORD inodePos) {
  LGA_LOGGER_DEBUG("[getSectorIndexInode] Getting");
  return floor(inodePos/INODE_PER_SECTOR) + INODE_SECTOR_INDEX;
}

int getOffsetInode(DWORD inodePos) {
  int inodeSectorPos = getSectorIndexInode(inodePos);
  return inodePos - (INODE_PER_SECTOR * (inodeSectorPos - INODE_SECTOR_INDEX));
}

void cleanArray(char *array, int size) {
  int i;

  for(i=0; i < size; i++) {
    array[i] = 0;
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
    return FIRST_ENTRY;
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
    return SECOND_ENTRY;
  }
  ///TODO IND E DOUB IND
}

int _printEntries(DWORD ptr) {
  char diskBuffer[BLOCK_SIZE_BYTES], registerBuffer[REGISTER_SIZE];

  if (readBlock(ptr, diskBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_printEntries] couldnt read the entry");
    return FAILED;
  }

  for (int i = 0; i < REGISTERS_PER_BLOCK; i++) {
    if (getRegisterFile(i, diskBuffer, BLOCK_SIZE_BYTES, registerBuffer) !=SUCCEEDED) {
      LGA_LOGGER_ERROR("[_printEntries] couldnt get the register file");
      return FAILED;
    }
    if (strlen(((FileRecord*) registerBuffer)->name) > 0) {
      if (((FileRecord*) registerBuffer)->TypeVal == TYPEVAL_REGULAR) {
        printf(".%s  ",((FileRecord*) registerBuffer)->name);
      } else if (((FileRecord*) registerBuffer)->TypeVal == TYPEVAL_DIRETORIO) {
        printf("./%s  ",((FileRecord*) registerBuffer)->name);
      }
    }
  }

  return SUCCEEDED;
}

void concatCustom(char* concatened, int concatStartPos, char* buffer, int bufferSize) {
  int i;
  for(i=0; i<bufferSize; i++) {
    concatened[concatStartPos + i] = buffer[i];
  }
}

void changeDisk(int start, char* data, int dataSize, char* diskSector, int diskSize, char* saveSector) {
  int i;
  LGA_LOGGER_DEBUG("[changeSectorInode] Changing the Sector");
  for(i = 0; i < diskSize; i++) {
    if (i >= start && i < dataSize+start) {
      saveSector[i] = data[i - start];
      printf("%c",saveSector[i] );
    } else {
      saveSector[i] = diskSector[i];
      printf("%c",saveSector[i] );

    }
  }
}

int getDataFromDisk(char *buffer, int start, int dataSize, char* diskBuffer, int diskSize) {
  if (dataSize > diskSize) {
    LGA_LOGGER_ERROR("[getDataSector] DATA size greater than SECTOR size");
    return FAILED;
  }

  for (int i=0; i < dataSize; i++) {
    buffer[i] = diskBuffer[start + i];
  }
  LGA_LOGGER_DEBUG("[getDataSector] Successfully");
  return SUCCEEDED;
}

int alreadyExists(char* filename, Inode inode) {
  if (inode.dataPtr[0] != INVALID_PTR) {
   if (_alreadyExists(inode.dataPtr[0], filename) != NOT_FOUND) {
      LGA_LOGGER_WARNING("[alreadyExists] couldnt verify the first entry");
      return FAILED;
    }
  }
  if (inode.dataPtr[1] != INVALID_PTR) {
   if (_alreadyExists(inode.dataPtr[1], filename) != NOT_FOUND) {
      LGA_LOGGER_WARNING("[alreadyExists] couldnt verify the second entry");
      return FAILED;
    }
  }
  return NOT_FOUND;
}

int _alreadyExists(DWORD ptr, char* filename) {
  char diskBuffer[BLOCK_SIZE_BYTES], registerBuffer[REGISTER_SIZE];

  if (readBlock(ptr, diskBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_alreadyExists] couldnt read the entry");
    return FAILED;
  }

  for (int i = 0; i < REGISTERS_PER_BLOCK; i++) {
    if (getRegisterFile(i, diskBuffer, BLOCK_SIZE_BYTES, registerBuffer) !=SUCCEEDED) {
      LGA_LOGGER_ERROR("[_alreadyExists] couldnt get the register file");
      return FAILED;
    }
    if (strcmp(((FileRecord*) registerBuffer)->name, filename) == 0) {
      return FOUND;
    }
  }
  return NOT_FOUND;
}
