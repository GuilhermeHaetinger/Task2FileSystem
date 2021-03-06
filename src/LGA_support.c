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
int FIRST_REG            = 0;
int SECOND_REG           = 0;
int SINGLE_REG           = 0;
int DOUBLE_REG           = 0;
int SINGLE_PTR           = 0;
int DOUBLE_PTR           = 0;
int PTR_PER_BLOCK        = 0;
/* ################################ */
/* --------- SUPER_BLOCK_SECTION  ---------- */
/* ################################ */

int initializeSuperBlock(){
  if(!superBlockRead){
    LGA_LOGGER_DEBUG("Superblock wasn't read yet");
    initializeOpenFilesAndDirectories();
    if(readSuperblock() != SUCCEEDED){
      LGA_LOGGER_ERROR("[initializeSystem] superBlock not read properly");
      return FAILED;
    }
    if(getRootInodeFile((char *)&openDirectory, (char *)&openDirectoryFileRecord) != SUCCEEDED){
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

    PTR_PER_BLOCK = BLOCK_SIZE_BYTES/sizeof(DWORD);

    FIRST_REG = REGISTERS_PER_BLOCK - 1;
    SECOND_REG = (REGISTERS_PER_BLOCK * 2) - 1;

    SINGLE_PTR = (SECOND_REG + 1) + (PTR_PER_BLOCK * REGISTERS_PER_BLOCK) - 1;
    DOUBLE_PTR = (SINGLE_PTR + 1) +(PTR_PER_BLOCK * PTR_PER_BLOCK * REGISTERS_PER_BLOCK) - 1;

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

int initializeOpenFilesAndDirectories(){
  closedRecord.TypeVal = TYPEVAL_INVALIDO;
  int i;
  recordHandler nullRecord;
  nullRecord.CP = 0;
  nullRecord.file = closedRecord;
  for(i = 0; i<MAX_NUM_OF_OPEN_FILES; i++){
    openFiles[i] = nullRecord;
  }
  closedDir.blocksFileSize = -1;
  closedDir.bytesFileSize  = -1;
  int closedEntry          = -1;
  directoryHandler closedHandler;
  closedHandler.dir = closedDir;
  closedHandler.entry = closedEntry;
  for(i = 0; i<MAX_NUM_OF_OPEN_DIRECTORIES; i++){
    openDirectories[i] = closedHandler;
  }
  return SUCCEEDED;
}

int createRecord(char * name, BYTE typeVal, FileRecord * file){
  LGA_LOGGER_DEBUG("[createRecord] Entering createRecord");
  FileRecord inodeAux;
  int auxPosition, accessedPtr;
  if (getFileInode(name, openDirectory, &inodeAux,&auxPosition, &accessedPtr) != NOT_FOUND) {
    LGA_LOGGER_WARNING("[createRecord] Already exist a file with that name");
    return FAILED;
 	}

	LGA_LOGGER_LOG("[createRecord] Searching for inode position");

	DWORD inodePos = getFreeInode();
  if((int)inodePos <= 0){
    LGA_LOGGER_WARNING("[createRecord] exceeded the maximum amount of inodes blocked");
    return FAILED;
  }

	file->TypeVal = typeVal;
  strncpy(file->name, name, 59);
	file->inodeNumber = inodePos;
  return SUCCEEDED;
}

int createRecordInode(FileRecord file){
  LGA_LOGGER_DEBUG("[createRecordInode] Entering CreatingRecordInode");
  Inode fileInode;
	initializeInode(&fileInode);

	if(saveInode(file.inodeNumber, (char *)&fileInode) != 0){
		LGA_LOGGER_ERROR("[createRecordInode] Inode not saved properly");
		return FAILED;
	}
  return SUCCEEDED;
}

int createDirectoryInode(FileRecord file, int fatherInodeNumber) {
  LGA_LOGGER_DEBUG("[createDirectoryInode] Entering CreatingRecordInode");
  Inode fileInode;
	initializeInode(&fileInode);

  FileRecord dot, dotdot; /* . && .. */
  dot.TypeVal = TYPEVAL_DIRETORIO;
  strcpy(dot.name, ".");
  dot.inodeNumber = file.inodeNumber;
  dotdot.TypeVal = TYPEVAL_DIRETORIO;
  strcpy(dotdot.name, "..");
  dotdot.inodeNumber = fatherInodeNumber;

  int sectorPos = searchBitmap2(BITMAP_DADOS, 0);

  char registersData[REGISTER_SIZE * 2];

  cleanBlock(sectorPos);
  changeWriteBlock(sectorPos, 0,(char*)&dot, REGISTER_SIZE);
  changeWriteBlock(sectorPos, REGISTER_SIZE,(char*)&dotdot, REGISTER_SIZE );
  fileInode.dataPtr[0] = sectorPos;
  fileInode.bytesFileSize +=  REGISTER_SIZE * 2;
  fileInode.blocksFileSize = 1;

	if(saveInode(file.inodeNumber, (char *)&fileInode) != 0){
		LGA_LOGGER_ERROR("[createDirectoryInode] Inode not saved properly");
		return FAILED;
	}


  return SUCCEEDED;
}

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
    LGA_LOGGER_WARNING("File won't be added to vector since it's full");
    return FAILED;
  }

  LGA_LOGGER_LOG("recordHandler being created");
  recordHandler record;
  record.file = file;
  record.CP = 0;

  FILE2 available_pos = findProperPositionOnOpenFiles();
  LGA_LOGGER_LOG("recordHandler being added");

  openFiles[available_pos] = record;

  LGA_LOGGER_LOG("handler being increased");
  openFilesHandler++;

  return available_pos;

}

int removeFileFromOpenFiles(FILE2 handler){
  recordHandler nullHandler;
  if(handler < MAX_NUM_OF_OPEN_FILES && handler >= 0){
    nullHandler.CP = 0;
    nullHandler.file = closedRecord;
    openFiles[handler] = nullHandler;

    openFilesHandler--;
    if (openFilesHandler < 0) openFilesHandler = 0;
    LGA_LOGGER_DEBUG("Handler decreased and position set to NULL");
    return SUCCEEDED;

  }else{
    LGA_LOGGER_ERROR("There is no file open with this handler");
    return FAILED;
  }
}

int addFileToOpenDirectory(FileRecord file){
  int accessedPtr = -1, newBlock = 0;
  int position = getNewFilePositionOnOpenDirectory(&accessedPtr, &newBlock);
  if (position < 0) {
    LGA_LOGGER_ERROR("[addFileToOpenDirectory] Couldnt add file to directory");
    return FAILED;
  }
  if (writeFilePositionInInode(openDirectory, (char*)&file, position) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[addFileToOpenDirectory] Couldnt write in inode");
    return FAILED;
  }
  LGA_LOGGER_LOG("[addFileToOpenFiles] Success");

  if (newBlock) {
    //Seta como ocupado o bloco livre obtido
    setBitmap2(BLOCK_TYPE,openDirectory.dataPtr[accessedPtr],1);
  }
  if (setInode(openDirectoryFileRecord.inodeNumber, (char*)&openDirectory) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[addFileToOpenFiles] Couldnt set inode");
    return FAILED;
  }
  return SUCCEEDED;
}

int getNewFilePositionOnOpenDirectory(int *accessedPtr,int *newBlock){

  //Procura no bloco de registros apontado por dataPtr[0] algum registro valido/livre pra usar
  int try = searchNewFileRecordPosition(&(openDirectory.dataPtr[0]),newBlock);
  if (try >= 0) {
    LGA_LOGGER_LOG("[getNewFilePositionOnOpenDirectory] Success dataPtr[0]");
    *accessedPtr = 0;
    return try;
  } else if (try == FAILED) {
    LGA_LOGGER_ERROR("[getNewFilePositionOnOpenDirectory] Couldnt get the position dataptr0");
    return FAILED;
  }

  //Caso cheio o ptr[0] procura no bloco de registros apontado por dataPtr[1]
  try = searchNewFileRecordPosition(&(openDirectory.dataPtr[1]),newBlock);
  if (try >= 0) {
    LGA_LOGGER_LOG("[getNewFilePositionOnOpenDirectory] Success dataPtr[1]");
    *accessedPtr = 1;
    return try + (REGISTERS_PER_BLOCK * 1);
  } else if (try == FAILED) {
    LGA_LOGGER_ERROR("[getNewFilePositionOnOpenDirectory] Couldnt get the position dataptr1");
    return FAILED;
  }

  try = singleIndGetPos(&(openDirectory.singleIndPtr),newBlock);
  if (try >= 0) {
    LGA_LOGGER_LOG("[getNewFilePositionOnOpenDirectory] Success singleIndGetPos");
    *accessedPtr = 1;
    return try;
  } else if (try == FAILED) {
    LGA_LOGGER_ERROR("[getNewFilePositionOnOpenDirectory] Couldnt get the position singleInd");
    return FAILED;
  }
  try = doubleIndGetPos(&(openDirectory.doubleIndPtr),newBlock);
  if (try >= 0) {
    LGA_LOGGER_LOG("[getNewFilePositionOnOpenDirectory] Success doubleIndGetPos");
    *accessedPtr = 1;
    return try;
  } else if (try == FAILED) {
    LGA_LOGGER_ERROR("[getNewFilePositionOnOpenDirectory] Couldnt get the position doubleInd");
    return FAILED;
  }
  LGA_LOGGER_WARNING("[getNewFilePositionOnOpenDirectory] Is full");
  return FAILED;
}

int searchNewFileRecordPosition(DWORD *ptr,int *newBlock) {
  //Se o ponteiro estava invalid (desalocado), procura um bloco livre disponivel
  if (*ptr == INVALID_PTR) {
    LGA_LOGGER_LOG("[searchNewFileRecordPosition] Invalid Ptr");
    *ptr = getFreeBlock();

    //Se nao há blocos livres disponiveis retorna erro
    if(*ptr < 0){
      LGA_LOGGER_ERROR("[searchNewFileRecordPosition] No available blocks");
      return FAILED;
    }
    setBitmap2(BLOCK_TYPE, *ptr , BLOCK_BUSY);
    openDirectory.blocksFileSize += 1;
    *newBlock = 1;
  }


  //Cria os buffers para ler o bloco e mexer nos dados
  char blockBuffer[BLOCK_SIZE_BYTES], registerBuffer[REGISTER_SIZE];

  //Le o bloco apontado pelo ptr
  readBlock(*ptr, blockBuffer, BLOCK_SIZE_BYTES);

  //Procura pelos registro de diretorios 1 a 1 no bloco em busca de um disponivel
  int position = 0;
  for (position = 0; position < BLOCK_SIZE_BYTES/REGISTER_SIZE; position++) {
    if (getRegisterFile(position, blockBuffer, BLOCK_SIZE_BYTES, registerBuffer) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[searchNewFileRecordPosition] Couldnt get the register");
      return FAILED;
      //TODO sepa nao seria return Failed, poderia prosseguir e setar como defeituoso(?)
    }

    //Quando achar um registro de diretorio disponivel o retorna
    if(((FileRecord*)registerBuffer)->TypeVal == TYPEVAL_INVALIDO) {
      LGA_LOGGER_LOG("[searchNewFileRecordPosition] Get the position");
      return position;
    }
  }
  LGA_LOGGER_DEBUG("[searchNewFileRecordPosition] Couldnt get the position");
  return BLOCK_FULL;
}

int _searchNewFileRecordPosition(DWORD ptr,int *newBlock) {
  //Cria os buffers para ler o bloco e mexer nos dados

  char blockBuffer[BLOCK_SIZE_BYTES], registerBuffer[REGISTER_SIZE];

  //Le o bloco apontado pelo ptr
  readBlock(ptr, blockBuffer, BLOCK_SIZE_BYTES);
  int position = 0;
  //Procura pelos registro de diretorios 1 a 1 no bloco em busca de um disponivel
  for (position = 0; position < BLOCK_SIZE_BYTES/REGISTER_SIZE; position++) {
    if (getRegisterFile(position, blockBuffer, BLOCK_SIZE_BYTES, registerBuffer) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[_searchNewFileRecordPosition] Couldnt get the register");
      return FAILED;
      //TODO sepa nao seria return Failed, poderia prosseguir e setar como defeituoso(?)
    }

    //Quando achar um registro de diretorio disponivel o retorna
    if(((FileRecord*)registerBuffer)->TypeVal == TYPEVAL_INVALIDO) {
      LGA_LOGGER_LOG("[_searchNewFileRecordPosition] Get the position");
      return position;
    }
  }
  LGA_LOGGER_DEBUG("[_searchNewFileRecordPosition] Couldnt get the position");
  return -3;
}

int printAllEntries(Inode inode) {


  if (inode.dataPtr[0] != INVALID_PTR) {
    if (_printEntries(inode.dataPtr[0]) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[printAllEntries] couldnt print the first entry");
      return FAILED;
    }
  }
  if (inode.dataPtr[1] != INVALID_PTR) {
    if (_printEntries(inode.dataPtr[1]) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[printAllEntries] couldnt print the second entry");
      return FAILED;
    }

  }
  if (inode.singleIndPtr != INVALID_PTR) {
    if (singleIndPrint(inode.singleIndPtr) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[printAllEntries] couldnt print the single ind entry");
      return FAILED;
    }
  }
  if (inode.doubleIndPtr != INVALID_PTR) {
    if (doubleIndPrint(inode.doubleIndPtr) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[printAllEntries] couldnt print the single ind entry");
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

  if (position <= FIRST_REG) {
    //TODO
    //Faz sentido passar position que foi multiplicada por REGISTERS_PER_BLOCK para o changeWriteBlock como dataPos?
    if (changeWriteBlock(inode.dataPtr[0], position * REGISTER_SIZE, fileRecord, REGISTER_SIZE) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[writeFilePositionInInode] Fail dataPtr0");
      return FAILED;
    }

    openDirectory.bytesFileSize += REGISTER_SIZE;
    LGA_LOGGER_LOG("[writeFilePositionInInode] Success dataptr0");
    return SUCCEEDED;
  } else if (position <= SECOND_REG) {

    if (changeWriteBlock(inode.dataPtr[1], (position - (FIRST_REG+1))* REGISTER_SIZE, fileRecord, REGISTER_SIZE) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[writeFilePositionInInode] Fail dataPtr1");
      return FAILED;
    }
    openDirectory.bytesFileSize += REGISTER_SIZE;
    LGA_LOGGER_LOG("[writeFilePositionInInode] Success dataPtr1");
    return SUCCEEDED;
  } else if (position <= SINGLE_PTR) {
    if (singleIndWrite(inode.singleIndPtr, (position - (SECOND_REG+1)), fileRecord) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[writeFilePositionInInode] Fail SingleInd");
      return FAILED;
    }
    LGA_LOGGER_LOG("[writeFilePositionInInode] Success singleInd");
    return SUCCEEDED;
  } else if (position <= DOUBLE_PTR) {
    if (doubleIndWrite(inode.doubleIndPtr, (position - (SINGLE_PTR+1)), fileRecord) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[writeFilePositionInInode] Fail SingleInd");
      return FAILED;
    }
    LGA_LOGGER_LOG("[writeFilePositionInInode] Success doubleInd");
    return SUCCEEDED;
  }
  LGA_LOGGER_ERROR("[writeFilePositionInInode] Couldnt write any");
  return FAILED;
}

int getSpecificEntry(Inode dir, int entryNum, char* buffer){
  bool isEntryInPtr1  = entryNum < REGISTERS_PER_BLOCK;
  bool isEntryInPtr2  = REGISTERS_PER_BLOCK <= entryNum && entryNum < 2* REGISTERS_PER_BLOCK;
  char diskBuffer[BLOCK_SIZE_BYTES];

  if(isEntryInPtr1){
    if (readBlock(dir.dataPtr[0], diskBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_WARNING("[getSpecificEntry] couldnt read the Block from dataPtr 0");
      return FAILED;
    }

  }else if(isEntryInPtr2){
    if (readBlock(dir.dataPtr[1], diskBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_WARNING("[getSpecificEntry] couldnt read the Block from dataPtr 1");
      return FAILED;
    }
    entryNum = entryNum - REGISTERS_PER_BLOCK;
  } else if(entryNum < 2 * REGISTERS_PER_BLOCK + PTR_PER_BLOCK * REGISTERS_PER_BLOCK) {
    if (getSpecificEntry_SingleInd(dir.singleIndPtr, entryNum - SECOND_REG + 1, buffer) != SUCCEEDED) {
      LGA_LOGGER_WARNING("[getSpecificEntry] couldnt read the Block from singleInd");
      return FAILED;
    }
    return SUCCEEDED;
  } else if (entryNum < 2 * REGISTERS_PER_BLOCK + PTR_PER_BLOCK * REGISTERS_PER_BLOCK + PTR_PER_BLOCK * PTR_PER_BLOCK * REGISTERS_PER_BLOCK) {
    if (getSpecificEntry_DoubleInd(dir.doubleIndPtr, entryNum - SINGLE_PTR+1, buffer) != SUCCEEDED) {
      LGA_LOGGER_WARNING("[getSpecificEntry] couldnt read the Block from doubleInd");
      return FAILED;
    }
    return SUCCEEDED;
  }
  if (getRegisterFile(entryNum, diskBuffer, BLOCK_SIZE_BYTES, buffer) !=SUCCEEDED) {
      LGA_LOGGER_ERROR("[getSpecificEntry] couldnt get the entry");
      return FAILED;
    }
    return SUCCEEDED;
}

int invalidateFromCPOn(DWORD CP, Inode  *fileInode){
  float  CPLocation = CP / (BLOCK_SIZE_BYTES);
  float CPBlock = floor(CP / BLOCK_SIZE_BYTES);

  if(CP == 0){
    LGA_LOGGER_DEBUG("INVALIDATING FROM CP = 0");
    if(fileInode->dataPtr[0] != INVALID_PTR) setBitmap2(BLOCK_TYPE, fileInode->dataPtr[0], 0);
    fileInode->dataPtr[0] = INVALID_PTR;
    if(fileInode->dataPtr[1] != INVALID_PTR) setBitmap2(BLOCK_TYPE, fileInode->dataPtr[1], 0);
    fileInode->dataPtr[1] = INVALID_PTR;
    if(fileInode->singleIndPtr != INVALID_PTR){
      setBitmap2(BLOCK_TYPE, fileInode->singleIndPtr, 0);
      singleIndInvalidate(fileInode->singleIndPtr, CPBlock, (BLOCK_SIZE_BYTES * 2)/BLOCK_SIZE_BYTES);
    }
    fileInode->singleIndPtr = INVALID_PTR;
    if(fileInode->doubleIndPtr != INVALID_PTR){
      setBitmap2(BLOCK_TYPE, fileInode->doubleIndPtr, 0);
      doubleIndInvalidate(fileInode->doubleIndPtr, CPBlock);
    }
    fileInode->doubleIndPtr = INVALID_PTR;
    return SUCCEEDED;
  }
  //CP In dataPtr[0]
  if(CP <= BLOCK_SIZE_BYTES){
    LGA_LOGGER_DEBUG("INVALIDATING FROM PTR[0]");
    if(fileInode->dataPtr[1] != INVALID_PTR) setBitmap2(BLOCK_TYPE, fileInode->dataPtr[1], 0);
    fileInode->dataPtr[1] = INVALID_PTR;
    if(fileInode->singleIndPtr != INVALID_PTR){
      setBitmap2(BLOCK_TYPE, fileInode->singleIndPtr, 0);
      singleIndInvalidate(fileInode->singleIndPtr, CPBlock, (BLOCK_SIZE_BYTES * 2)/BLOCK_SIZE_BYTES);
    }
    fileInode->singleIndPtr = INVALID_PTR;
    if(fileInode->doubleIndPtr != INVALID_PTR){
      setBitmap2(BLOCK_TYPE, fileInode->doubleIndPtr, 0);
      doubleIndInvalidate(fileInode->doubleIndPtr, CPBlock);
    }
    fileInode->doubleIndPtr = INVALID_PTR;
    return SUCCEEDED;
  }
  //CP In dataPtr[1]
  if(CP <= 2 * BLOCK_SIZE_BYTES){
    LGA_LOGGER_DEBUG("INVALIDATING FROM PTR[1]");
    if(fileInode->singleIndPtr != INVALID_PTR){
      setBitmap2(BLOCK_TYPE, fileInode->singleIndPtr, 0);
      singleIndInvalidate(fileInode->singleIndPtr, CPBlock, (BLOCK_SIZE_BYTES * 2)/BLOCK_SIZE_BYTES);
    }
    fileInode->singleIndPtr = INVALID_PTR;
    if(fileInode->doubleIndPtr != INVALID_PTR){
      setBitmap2(BLOCK_TYPE, fileInode->doubleIndPtr, 0);
      doubleIndInvalidate(fileInode->doubleIndPtr, CPBlock);
    }
    fileInode->doubleIndPtr = INVALID_PTR;
    return SUCCEEDED;
  }
  if(CP <= 2 * BLOCK_SIZE_BYTES + PTR_PER_BLOCK*BLOCK_SIZE_BYTES){
    LGA_LOGGER_DEBUG("INVALIDATING FROM SINGLE IND");
    fileInode->bytesFileSize = CP;
    if(fileInode->singleIndPtr != INVALID_PTR){
      singleIndInvalidate(fileInode->singleIndPtr, CPBlock, (BLOCK_SIZE_BYTES * 2)/BLOCK_SIZE_BYTES);
    }
    if(fileInode->doubleIndPtr != INVALID_PTR){
      setBitmap2(BLOCK_TYPE, fileInode->doubleIndPtr, 0);
      doubleIndInvalidate(fileInode->doubleIndPtr, CPBlock);
    }
    fileInode->doubleIndPtr = INVALID_PTR;
    return SUCCEEDED;
  }
  else{
    LGA_LOGGER_DEBUG("INVALIDATING FROM DOUBLE IND");
    if(fileInode->doubleIndPtr != INVALID_PTR){
      doubleIndInvalidate(fileInode->doubleIndPtr, CPBlock);
    }
    return SUCCEEDED;
  }
  return SUCCEEDED;
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

  return SUCCEEDED;
}

int readBlock(int blockPos, char* data, int dataSize){

  int sectorPos = blockPos * SECTORS_PER_BLOCK;

  if (dataSize != BLOCK_SIZE_BYTES) {
    LGA_LOGGER_ERROR("[readBlock] Data size is different from Block size");
    return FAILED;
  }

  int i, j;

  LGA_LOGGER_DEBUG("[readBlock] Entering reading loop");

  char sectorBuffer[SECTOR_SIZE];

  for(i = 0; i < superBlock.blockSize; i++){
    if(read_sector(sectorPos + i, sectorBuffer) != SUCCEEDED){
      LGA_LOGGER_ERROR("[readBlock] reading failed in reading loop");
      return FAILED;
    }
    for(j = 0; j < SECTOR_SIZE; j++){
      data[j + i * SECTOR_SIZE] = sectorBuffer[j];
    }
  }
  LGA_LOGGER_DEBUG("[readBlock] Block read successfully");
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

  //Testa que a posicao do inode estava livre no bitmap
  LGA_LOGGER_DEBUG("[saveInode] Setting iNode to busy on bitmap");
  if(getBitmap2(INODE_TYPE, inodePos) == INODE_BUSY){
    LGA_LOGGER_ERROR("[saveInode] Inode Bitmap positionis already busy");
    return FAILED;
  }

  //TODO talvez testar se consegue lockar direito inode
  if (setBitmap2(INODE_TYPE, inodePos, INODE_BUSY) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[saveInode] Couldnt set bitmap");
    return FAILED;
  }
  //Obtem o setor onde se encontra o inode
  int inodeSectorPos = getSectorIndexInode(inodePos);

  //Lê o que tem no setor inteiro onde se encontra também nosso inode
  char diskSector[SECTOR_SIZE];
  LGA_LOGGER_DEBUG("[saveInode] Reading inode sector");
  if(read_sector(inodeSectorPos, diskSector) != SUCCEEDED){
    LGA_LOGGER_ERROR("[saveInode] inode sector not read properly");
    return FAILED;
  }
  LGA_LOGGER_DEBUG("[saveInode] inode sector read properly");

  //Calcula offset no setor para calcular inicio do nosso inode
  int offset = getOffsetInode(inodePos);

  //Alter o setor modificando o inode passado e inserindo o data passado
  char sectorData[SECTOR_SIZE];
  changeSectorInode(offset, data, INODE_SIZE, diskSector, sectorData);
  LGA_LOGGER_DEBUG("[saveInode] writing new inode block");

  //Atualiza o setor com o inode contendo o data e o resto de dados originais
  write_sector(inodeSectorPos, sectorData);
  LGA_LOGGER_LOG("[saveInode] Successfully");
  return SUCCEEDED;
}

int setInode(DWORD inodePos, char* data){


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
    LGA_LOGGER_ERROR("[getInode]inode sector not read properly");
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

int removeInode(DWORD inodePos) {
  char inode[INODE_SIZE];
  getInode(inodePos, inode);

  if (((Inode*)inode)->dataPtr[0] != INVALID_PTR) {
    if (setBitmap2(BLOCK_TYPE, ((Inode*)inode)->dataPtr[0], BLOCK_FREE) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[removeInode] Couldnt set bitmap2 dataptr0");
      return FAILED;
    }
  }
  if (((Inode*)inode)->dataPtr[1] != INVALID_PTR) {
    if (setBitmap2(BLOCK_TYPE, ((Inode*)inode)->dataPtr[1], BLOCK_FREE) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[removeInode] Couldnt set bitmap2 dataptr1");
      return FAILED;
    }
  }
  if (((Inode*)inode)->singleIndPtr != INVALID_PTR) {
    if (_removeInode_SingleInd(((Inode*)inode)->singleIndPtr) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[removeInode] Couldnt _removeInode_SingleInd");
      return FAILED;
    }
  }
  if (((Inode*)inode)->doubleIndPtr != INVALID_PTR) {
    if (_removeInode_DoubleInd(((Inode*)inode)->doubleIndPtr) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[removeInode] Couldnt _removeInode_DoubleInd");
      return FAILED;
    }
  }

  if (setBitmap2(INODE_TYPE, inodePos, INODE_FREE) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[removeInode] Couldnt set bitmap2 inode");
    return FAILED;
  }
  return SUCCEEDED;
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

  setBitmap2(INODE_TYPE, ROOT_INODE, INODE_BUSY);
  LGA_LOGGER_DEBUG("[createRoot] Setting iNode to busy on bitmap");
  setInode(ROOT_INODE, (char *)&rootInode);
  LGA_LOGGER_LOG("[createRoot] Root created");
  return SUCCEEDED;
}

int getRootInodeFile(char* inodeBuffer, char* fileBuffer) {

  if (rootCreated() != SUCCEEDED) {
    if (createRoot() != SUCCEEDED) {
      LGA_LOGGER_ERROR("[getRoot] There's no Root");
      return FAILED;
    }
  }

  if (getInode(ROOT_INODE, inodeBuffer) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getRoot] Root iNode couldnt be read");
    return FAILED;
  }

  FileRecord rootFileRecord;
  rootFileRecord.TypeVal = TYPEVAL_DIRETORIO;
  rootFileRecord.inodeNumber = ROOT_INODE;
  strcpy(rootFileRecord.name, "root");
  concatCustom(fileBuffer, 0, (char*) &rootFileRecord, REGISTER_SIZE);

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
/* --------   DIRECTORY-SECTION  ------- */
/* ################################ */

int setNewOpenDirectory(char * directoryName){
  FileRecord dir;
  int auxPosition, accessedPtr;
  if (getFileInode(directoryName, openDirectory, &dir, &auxPosition, &accessedPtr) == NOT_FOUND) {
    LGA_LOGGER_WARNING("[setNewOpenDirectory]Directory not found in this directory");
    return FAILED;
  }else{
    Inode dirInode;
    if(getInode(dir.inodeNumber, (char * )&dirInode) != SUCCEEDED){
      LGA_LOGGER_ERROR("[setNewOpenDirectory]Couldn't read new directory's inode");
      return FAILED;
    }else{
      LGA_LOGGER_DEBUG("[setNewOpenDirectory]Changed the directory");
      openDirectory = dirInode;
      openDirectoryFileRecord = dir;
      return SUCCEEDED;
    }
  }
}

DIR2 addDirToOpenDirs(Inode dir){
  DIR2 available_pos = findProperPositionOnOpenDirectories();
  LGA_LOGGER_LOG("[addDirToOpenDirs]directory being added");

  directoryHandler handler;
  handler.entry = 0;
  handler.dir   = dir;
  openDirectories[available_pos] = handler;

  LGA_LOGGER_LOG("[addDirToOpenDirs]handler being increased");
  openDirectoriesHandler++;

  return available_pos;

}

int removeDirFromOpenDirs(DIR2 handler){
  if(handler >= 0){
    directoryHandler closedHandler;
    closedHandler.dir = closedDir;
    closedHandler.entry = -1;
    openDirectories[handler] = closedHandler;

    openDirectoriesHandler--;
    if (openDirectoriesHandler < 0) openDirectoriesHandler = 0;
    LGA_LOGGER_DEBUG("Handler decreased and position set to NULL");
    return SUCCEEDED;

  }else{
    LGA_LOGGER_ERROR("There is no directory open with this handler");
    return FAILED;
  }
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

///NOT USED
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
    return FIRST_REG;
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
    return SECOND_REG;
  }
  ///TODO IND E DOUB IND
}

int _printEntries(DWORD ptr) {
  char diskBuffer[BLOCK_SIZE_BYTES], registerBuffer[REGISTER_SIZE];
  if(ptr == INVALID_PTR) {
    return SUCCEEDED;
  }
  if (readBlock(ptr, diskBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_printEntries] couldnt read the entry");
    return FAILED;
  }
  int i = 0;
  for (i = 0; i < REGISTERS_PER_BLOCK; i++) {
    if (getRegisterFile(i, diskBuffer, BLOCK_SIZE_BYTES, registerBuffer) !=SUCCEEDED) {
      LGA_LOGGER_ERROR("[_printEntries] couldnt get the register file");
      return FAILED;
    }

    if (strlen(((FileRecord*) registerBuffer)->name) > 0) {
      if (((FileRecord*) registerBuffer)->TypeVal == TYPEVAL_REGULAR) {
        printf("%s %d  ",((FileRecord*) registerBuffer)->name, ((FileRecord*) registerBuffer)->inodeNumber);
      } else if (((FileRecord*) registerBuffer)->TypeVal == TYPEVAL_DIRETORIO) {
        printf("%s %d  /  ",((FileRecord*) registerBuffer)->name, ((FileRecord*) registerBuffer)->inodeNumber);
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
    } else {
      saveSector[i] = diskSector[i];

    }
  }
}

int getDataFromDisk(char *buffer, int start, int dataSize, char* diskBuffer, int diskSize) {
  if (dataSize > diskSize) {
    LGA_LOGGER_ERROR("[getDataSector] DATA size greater than SECTOR size");
    return FAILED;
  }
  int i = 0;
  for (i=0; i < dataSize; i++) {
    buffer[i] = diskBuffer[start + i];
  }
  //LGA_LOGGER_DEBUG("[getDataSector] Successfully");
  return SUCCEEDED;
}

int getFileInode(char* filename, Inode inode, FileRecord * fileInode, int *position, int *accessedPtr) {

  int searchResult;
  //Procura no dataPtr[0] do register pelo inode com nome do arquivo a pesquisar
  if (inode.dataPtr[0] != INVALID_PTR) {
    searchResult = _getFileInode(inode.dataPtr[0], filename, fileInode, position);
   if ( searchResult != NOT_FOUND) {
     if (searchResult == FOUND) {
        *accessedPtr = 0;
        LGA_LOGGER_LOG("[getFileInode] Found the inode");
        return FOUND;
     }
     else{
        LGA_LOGGER_WARNING("[getFileInode] Couldnt verify the first entry");
        return FAILED;
      }
    }
  }

  //Procura no dataPtr[1] do register pelo inode com nome do arquivo a pesquisar
  if (inode.dataPtr[1] != INVALID_PTR) {
    searchResult = _getFileInode(inode.dataPtr[1], filename, fileInode, position);
   if ( searchResult != NOT_FOUND) {
     if (searchResult == FOUND) {
        LGA_LOGGER_LOG("[getFileInode] Found the inode");
        *accessedPtr = 1;
        *position = *position + (REGISTERS_PER_BLOCK * 1);
        return FOUND;
     }
     else{
        LGA_LOGGER_WARNING("[getFileInode] Couldnt verify the second entry");
        return FAILED;
      }
    }
  }

  if (inode.singleIndPtr != INVALID_PTR) {
    searchResult = _getFileInodeSingInd(inode.singleIndPtr, filename, fileInode, position);
   if ( searchResult != NOT_FOUND) {
     if (searchResult == FOUND) {
        LGA_LOGGER_LOG("[getFileInode] Found the inode");
        *accessedPtr = 1;
        *position = *position + (REGISTERS_PER_BLOCK * 1);
        return FOUND;
     }
     else{
        LGA_LOGGER_WARNING("[getFileInode] Couldnt verify the singInd entry");
        return FAILED;
      }
    }
  }

  if (inode.doubleIndPtr != INVALID_PTR) {
    searchResult = _getFileInodeDoubleInd(inode.doubleIndPtr, filename, fileInode, position);
   if ( searchResult != NOT_FOUND) {
     if (searchResult == FOUND) {
        LGA_LOGGER_LOG("[getFileInode] Found the inode");
        *accessedPtr = 1;
        *position = *position + (REGISTERS_PER_BLOCK * 1);
        return FOUND;
     }
     else{
        LGA_LOGGER_WARNING("[getFileInode] Couldnt verify the doubleInd entry");
        return FAILED;
      }
    }
  }
  return NOT_FOUND;
}

int _getFileInode(DWORD ptr, char* filename, FileRecord * fileInode,int *position) {
  char diskBuffer[BLOCK_SIZE_BYTES], registerBuffer[REGISTER_SIZE];

  //Le o bloco apontado por ptr e coloca no buffer
  if (readBlock(ptr, diskBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_getFileInode] couldnt read the entry");
    return FAILED;
  }
  int i = 0;
  //Le de registro em registro dentro do buffer
  for ( i = 0; i < REGISTERS_PER_BLOCK; i++) {
    if (getRegisterFile(i, diskBuffer, BLOCK_SIZE_BYTES, registerBuffer) !=SUCCEEDED) {
      LGA_LOGGER_ERROR("[_getFileInode] couldnt get the register file");
      return FAILED;
    }

    //Pode ter sido deletada uma entrada do diretorio e setado para INVALIDO entao nao consideramos esse nome desse residuo lixo
    if(((FileRecord*) registerBuffer)->TypeVal != TYPEVAL_INVALIDO)
    {
      //Se registro possuir o nome passado é porque é o arquivo procurado
      if (strcmp(((FileRecord*) registerBuffer)->name, filename) == SUCCEEDED) {
        *fileInode = *((FileRecord*) registerBuffer);
        *position = i;
        return FOUND;
      }
    }
  }
  return NOT_FOUND;
}

int _getFileInodeSingInd(DWORD singleIndPtr, char* filename, FileRecord * fileInode,int *position) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0, searchResult = 0;

  if (singleIndPtr == INVALID_PTR) {
    return SUCCEEDED;
  }
  if (readBlock(singleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_getFileInodeSingInd] Couldnt read");
    return FAILED;
  }
  int i;
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[_getFileInodeSingInd] Couldnt getData");
      return FAILED;
    }

    if(*((DWORD*)ptrBuffer) != INVALID_PTR && *((DWORD*)ptrBuffer) != 0) {
      searchResult = _getFileInode(*((DWORD*)ptrBuffer), filename, fileInode, position);
      if (searchResult != NOT_FOUND) {
        if (searchResult == FOUND) {
          LGA_LOGGER_LOG("[_getFileInodeSingInd] Found the inode");
          return FOUND;
        } else {
           LGA_LOGGER_WARNING("[_getFileInodeSingInd] Couldnt verify the singind entry");
           return FAILED;
         }
      }
    }
  }
  return NOT_FOUND;
}

int _getFileInodeDoubleInd(DWORD doubleIndPtr, char* filename, FileRecord * fileInode,int *position) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0, searchResult = 0;

  if (doubleIndPtr == INVALID_PTR) {
    return SUCCEEDED;
  }
  if (readBlock(doubleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_getFileInodeDoubleInd] Couldnt read");
    return FAILED;
  }
  int i = 0;
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[_getFileInodeDoubleInd] Couldnt getData");
      return FAILED;
    }

    if(*((DWORD*)ptrBuffer) != TYPEVAL_INVALIDO && *((DWORD*)ptrBuffer) != 0) {
      searchResult = _getFileInodeSingInd(*((DWORD*)ptrBuffer), filename, fileInode, position);
      if (searchResult != NOT_FOUND) {
        if (searchResult == FOUND) {
          LGA_LOGGER_LOG("[_getFileInodeDoubleInd] Found the inode");
          return FOUND;
        } else {
           LGA_LOGGER_WARNING("[_getFileInodeDoubleInd] Couldnt verify the singind entry");
           return FAILED;
         }
      }
    }
  }
  return NOT_FOUND;
}

DWORD getDirFilenameInode(char* filename, Inode inode) {
  DWORD searchResult;
  if (inode.dataPtr[0] != INVALID_PTR) {
    searchResult = _getDirFilenameInode(inode.dataPtr[0], filename);
   if ( searchResult != (DWORD) INVALID_PTR) {
      LGA_LOGGER_LOG("[getDirFilenameInode] filename inode found");
      return searchResult;
    }
  }
  if (inode.dataPtr[1] != INVALID_PTR) {
    searchResult = _getDirFilenameInode(inode.dataPtr[1], filename);
   if ( searchResult != NOT_FOUND) {
      LGA_LOGGER_LOG("[getDirFilenameInode] filename inode found");
      return searchResult;
    }
  }
  return (DWORD) INVALID_PTR;
}


//Funcao auxiliar para procurar dentro de um bloco de inodes o que contenha o filename
DWORD _getDirFilenameInode(DWORD ptr, char* filename) {
  char diskBuffer[BLOCK_SIZE_BYTES], registerBuffer[REGISTER_SIZE];

  //Le o bloco apontado por ptr e coloca no buffer
  if (readBlock(ptr, diskBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_getDirFilenameInode] couldnt read the entry");
    return (DWORD) INVALID_PTR;
  }

  //Le de registro em registro dentro do buffer
  int i = 0;
  for (i = 0; i < REGISTERS_PER_BLOCK; i++) {
    if (getRegisterFile(i, diskBuffer, BLOCK_SIZE_BYTES, registerBuffer) !=SUCCEEDED) {
      LGA_LOGGER_ERROR("[_getDirFilenameInode] couldnt get the register file");
      return (DWORD) INVALID_PTR;
    }

    //Se registro possuir o nome passado é porque é o arquivo procurado
    if (strcmp(((FileRecord*) registerBuffer)->name, filename) == 0) {
      return ((FileRecord*) registerBuffer)->inodeNumber;
    }
  }
  return (DWORD) INVALID_PTR;
}

int findFileRecordOnDirectory(char * filename){
  FileRecord file;
  int auxPosition, accessedPtr;
  if (getFileInode(filename, openDirectory, &file, &auxPosition, &accessedPtr) == NOT_FOUND) {
    LGA_LOGGER_WARNING("File not found in this directory");
    return FAILED;
  }else{
    LGA_LOGGER_DEBUG("File found");
    return SUCCEEDED;
  }
}

FILE2 findProperPositionOnOpenFiles(){
  LGA_LOGGER_DEBUG("entered find proper open files position");
  int pos;
  for(pos = 0; pos <= openFilesHandler; pos++){
    if(openFiles[pos].file.TypeVal == TYPEVAL_INVALIDO){
      return pos;
    }
  }
  LGA_LOGGER_ERROR("couldnt find proper position");
  return FAILED;
}

DIR2 findProperPositionOnOpenDirectories(){
  LGA_LOGGER_DEBUG("entered find proper open dir position");
  int pos;
  for(pos = 0; pos <= openDirectoriesHandler; pos++){
    if(openDirectories[pos].entry == -1){
      return pos;
    }
  }
  return FAILED;
}

/// Receive one path string and fills a given ***char with each directory
/// Arguments
/// parse(string,...) being string a char[]
/// parse (...,list)  being list a char***
/// Returns number of strings filled in the list
/// Access list as an array of strings ("%s", list[1])
int parse (char * string, char ***lista) {

   const char token[2] = "/";
   char *tokenString;
   int words = 0, i = 0;

   ///Count number of / and len
   do
   {
      if(string[i] == '/')
      {
          words++;
      }
      i++;
   }while(string[i] != '\0');

   //Create a copy of the string for using with strtok
   char str[i];
   strcpy(str,string);

   ///If not empty path, then sum 1 for the last word if it hasn't /
   if(words > 0 && string[i-1]!= '/')
   {
        words++;
   }
   //Caso não se tenha achado nenhuma barra mas tenham caracteres é pq é uma palavra apenas
   else if (words == 0 && i > 0) {
     words = 1;
   }

   ///Allocate memory for the array of strings
   *lista = malloc(sizeof(char*) * words);
   i=0;

   //Se primeiro caractere é / é pq começa pelo raiz e strtok iria ignorar
   if (str[0] == '/') {
     (*lista)[0] = malloc(sizeof(char) * (strlen("/") + 1));
     strcpy((*lista)[0],"/");
     i++;
   }

    /// get the first substring
   tokenString = strtok(str, token);

   /// walk through other tokens
   while( tokenString != NULL ) {
    (*lista)[i] = malloc(sizeof(char) * (strlen(tokenString) + 1)); ///Allocate memory at the current index for a string of the len of the current tokenString
    strcpy((*lista)[i],tokenString);                                ///Copy actual tokenString to the allocated index
    i++;
    tokenString = strtok(NULL, token);

   }

    return words;
}

void freeList(char ***lista, int indexes)
{
  int i;
  for ( i = 0; i < indexes; i++) {
    free((*lista)[i]);
  }
  LGA_LOGGER_DEBUG("[freeList] Strings of the vector have been freed");
  free(*lista);
  LGA_LOGGER_DEBUG("[freeList] Vector of the list has been free'd");

}

///NOT USED
int writeOnFile(Inode * fileInode, int CP, char * content, int contentSize){

  int block, forwardoffset, backwardsoffset;
  char blockBuffer[BLOCK_SIZE_BYTES];
  int byte = CP, i;
  int writeBuf = contentSize;
  LGA_LOGGER_DEBUG("[writeOnFile] Started to write");
  while(writeBuf > 0){

    if(getByteBlockPositioning(fileInode, CP, &block, &forwardoffset, &backwardsoffset, contentSize, contentSize - writeBuf) != SUCCEEDED){
      LGA_LOGGER_ERROR("[WriteOnFile] couldnt get block");
      return FAILED;
    }
    if(readBlock(block, blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED){
      LGA_LOGGER_ERROR("[WriteOnFile] couldnt read block");
      return FAILED;
    }
    for(i = 0; i < BLOCK_SIZE_BYTES; i++){
      if(i >= backwardsoffset && i < BLOCK_SIZE_BYTES - forwardoffset){
        blockBuffer[i] = content[byte - CP];
        byte++;
      }
    }
    if(writeBlock(block, blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED){
      LGA_LOGGER_ERROR("[WriteOnFile] couldnt write block");
      return FAILED;
    }
    writeBuf -= -backwardsoffset + BLOCK_SIZE_BYTES - forwardoffset;
  }
  return SUCCEEDED;
}
///NOT USED
int getByteBlockPositioning(Inode * fileInode, int byte, int * block, int * forwardOffset, int * backwardsOffset, int contentSize, int contentWritten){
  float  byteLocation = byte / (BLOCK_SIZE_BYTES);

  //byte in 0
  if(byte == 0){

    LGA_LOGGER_DEBUG("[getByteBlockPositioning] Writing on a new dataptr[0]");
    if(fileInode->dataPtr[0] == INVALID_PTR && contentSize > 0){
      fileInode->dataPtr[0] = getFreeBlock();
      LGA_LOGGER_DEBUG("[getByteBlockPositioning] Created dataPtr[0]");
    }
    *block = fileInode->dataPtr[0];
    *forwardOffset = BLOCK_SIZE_BYTES - byte - contentSize;
    if(*forwardOffset < 0) *forwardOffset = 0;
    *backwardsOffset = byte;

    return SUCCEEDED;
  }
  //byte In dataPtr[0]
  if(byte + contentWritten < BLOCK_SIZE_BYTES){

    LGA_LOGGER_DEBUG("[getByteBlockPositioning] Writing on dataptr[0]");
    if(byte + contentSize > BLOCK_SIZE_BYTES && fileInode->dataPtr[1] == INVALID_PTR){
      fileInode->dataPtr[1] = getFreeBlock();
      LGA_LOGGER_DEBUG("[getByteBlockPositioning] Created dataPtr[1]");
    }
    *block = fileInode->dataPtr[0];
    *forwardOffset = BLOCK_SIZE_BYTES - byte - contentSize - contentWritten;
    if(*forwardOffset < 0) *forwardOffset = 0;
    *backwardsOffset = byte;

    return SUCCEEDED;
  }
  //byte In dataPtr[1]
  if(byte + contentWritten < 2 * BLOCK_SIZE_BYTES){

    LGA_LOGGER_DEBUG("[getByteBlockPositioning] Writing on dataptr[1]");
    if(byte + contentSize - BLOCK_SIZE_BYTES > BLOCK_SIZE_BYTES && fileInode->singleIndPtr == INVALID_PTR){
      fileInode->singleIndPtr = getFreeBlock();

      LGA_LOGGER_DEBUG("[getByteBlockPositioning] Created singleIndPtr");
    }
    *block = fileInode->dataPtr[1];
    *forwardOffset = BLOCK_SIZE_BYTES - byte - contentSize - contentWritten - BLOCK_SIZE_BYTES;
    if(*forwardOffset < 0) *forwardOffset = 0;
    *backwardsOffset = 0;

    return SUCCEEDED;
  }
  LGA_LOGGER_ERROR("Didnt enter on one");

  if(byte + contentWritten < BLOCK_SIZE_BYTES * PTR_PER_BLOCK + 2 * BLOCK_SIZE_BYTES){
  }
  else{

  }
}
///NOT USED
int readFileBlocks(Inode fileInode, int CP, char * buffer, int contentSize){
  LGA_LOGGER_DEBUG("Entering readFileBlocks");
  int byte;
  DWORD ptr = fileInode.dataPtr[0];
  char block[BLOCK_SIZE_BYTES];
  if(readBlock(ptr, block, BLOCK_SIZE_BYTES) != SUCCEEDED){
    LGA_LOGGER_ERROR("[readFileBlocks] Couldnt read ptr 0");
  }
  for(byte = CP; byte < contentSize + CP; byte++){
    if(byte >= fileInode.bytesFileSize){
      return byte;
    }
    if(byte < BLOCK_SIZE_BYTES){
      LGA_LOGGER_DEBUG("[readFileBlocks] Reading on dataptr[0]");
      buffer[byte - CP] = block[byte];
    }
    if(byte >= BLOCK_SIZE_BYTES){
      LGA_LOGGER_DEBUG("[readFileBlocks] Reading on dataptr[1]");
      if(ptr != fileInode.dataPtr[1]){
        ptr = fileInode.dataPtr[1];
      }
      if(readBlock(ptr, block, BLOCK_SIZE_BYTES) != SUCCEEDED){
        LGA_LOGGER_ERROR("[readFileBlocks] Couldnt read ptr 1");
      }
      buffer[byte - CP] = block[byte - BLOCK_SIZE_BYTES];
    }
    if(byte >= 2 * BLOCK_SIZE_BYTES){
      LGA_LOGGER_DEBUG("[readFileBlocks] Reading on singular indirection");
      if(ptr != fileInode.singleIndPtr){
      ptr = fileInode.singleIndPtr;
    }
    /*  if(readBlock(ptr, block, BLOCK_SIZE_BYTES) != SUCCEEDED){
        LGA_LOGGER_ERROR("[readFileBlocks] Couldnt read ptr 1");
      }*/
      return FAILED;
    }
    if(byte >= 2 * BLOCK_SIZE_BYTES + BLOCK_SIZE_BYTES * BLOCK_SIZE_BYTES){
      LGA_LOGGER_DEBUG("[readFileBlocks] Reading on double indirection");
      if(ptr != fileInode.doubleIndPtr){
        ptr = fileInode.doubleIndPtr;
      }
   /*   if(readBlock(ptr, block, BLOCK_SIZE_BYTES) != SUCCEEDED){
        LGA_LOGGER_ERROR("[readFileBlocks] Couldnt read ptr 1");
      }*/
      return FAILED;
    }

  }
  return contentSize;
}

// OK
int singleIndGetPos( DWORD *singleIndPtr, int *newBlock) {
  LGA_LOGGER_DEBUG("[singleIndGetPos] Verify if singleInd exists");

  if (*singleIndPtr == INVALID_PTR) {
    LGA_LOGGER_LOG("[singleIndGetPos] Allocating singleInd");
    *singleIndPtr = getFreeBlock();
    if (cleanBlock(*singleIndPtr) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[singleIndGetPos] Couldnt clean the singleInd block");
      return FAILED;
    }
    setBitmap2(BLOCK_TYPE, *singleIndPtr, BLOCK_BUSY);
    openDirectory.blocksFileSize += 1;
    return SECOND_REG + 1;
  }

  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0;
  int ptrPerBlock = (BLOCK_SIZE_BYTES/sizeof(DWORD));

  LGA_LOGGER_DEBUG("[singleIndGetPos] Reading Block");
  if (readBlock(*singleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[singleIndGetPos] Couldnt read");
    return FAILED;
  }
  int i = 0;
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    LGA_LOGGER_DEBUG("[singleIndGetPos] Getting Data");
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[singleIndGetPos] Couldnt getData");
      return FAILED;
    }
    LGA_LOGGER_DEBUG("[singleIndGetPos] Searching for position");
    if(*((DWORD*)ptrBuffer) != 0 && *((DWORD*)ptrBuffer) != INVALID_PTR) {
      LGA_LOGGER_DEBUG("[singleIndGetPos] ptrBuffer exists");
      try = _searchNewFileRecordPosition(*((DWORD*)ptrBuffer), newBlock);
      if (try >= 0) try = try + i * REGISTERS_PER_BLOCK;

    } else {
      LGA_LOGGER_LOG("[singleIndGetPos] Creating ptrBuffer");
      newNewBlock = getFreeBlock();
      if (cleanBlock(newNewBlock) != SUCCEEDED) {
        LGA_LOGGER_ERROR("[singleIndGetPos] Couldnt clean the register block");
        return FAILED;
      }
      setBitmap2(BLOCK_TYPE, newNewBlock, BLOCK_BUSY);
      changeWriteBlock(*singleIndPtr, i*sizeof(DWORD), (char*)&newNewBlock, sizeof(DWORD));
      openDirectory.blocksFileSize += 1;
      openDirectory.bytesFileSize += sizeof(DWORD);
      try = i * REGISTERS_PER_BLOCK;
    }
    if (try >= 0) return try + (SECOND_REG + 1);
  }
  return BLOCK_FULL;
}

// OK
int singleIndWrite(DWORD singleIndPtr, int position, char * fileRecord) {

  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)], tempBlockBuffer[BLOCK_SIZE_BYTES];
  int ptrPerBlock = (BLOCK_SIZE_BYTES/sizeof(DWORD));
  int ptrPosition = floor((position/REGISTERS_PER_BLOCK));
  int normalizedPtrPosition = position - (ptrPosition * REGISTERS_PER_BLOCK);
  int newBlock;

  if (readBlock(singleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[singleIndWrite] Couldnt read");
    return FAILED;
  }

  if (getDataFromDisk(ptrBuffer, ptrPosition*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[singleIndWrite] Couldnt getData");
    return FAILED;
  }

  // Se não existe o endereço do bloco desejado dentro do Single, cria ele
  if (*((DWORD*)ptrBuffer) == 0) {
    newBlock = getFreeBlock();
    if (cleanBlock(newBlock) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[singleIndWrite] Couldnt clean the singleInd block");
      return FAILED;
    }
    setBitmap2(BLOCK_TYPE,newBlock, BLOCK_BUSY);
    changeWriteBlock(singleIndPtr, ptrPosition, (char*)&newBlock, sizeof(DWORD));
    openDirectory.blocksFileSize += 1;
    openDirectory.bytesFileSize += sizeof(DWORD);
    strcpy(ptrBuffer,(char*)&newBlock);
  }

  // Escreve o registro dentro do bloco que está dentro do bloco do single.
  if (changeWriteBlock(*((DWORD*)ptrBuffer), normalizedPtrPosition * REGISTER_SIZE, fileRecord, REGISTER_SIZE) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[singleIndWrite] Couldnt changeWrite");
    return FAILED;
  }
    openDirectory.bytesFileSize += REGISTER_SIZE;
  LGA_LOGGER_LOG("[singleIndWrite] Sucess");
  return SUCCEEDED;
}

// OK
int doubleIndGetPos( DWORD *doubleIndPtr, int *newBlock) {
  LGA_LOGGER_DEBUG("[doubleIndGetPos] Verify if singleInd exists");

  if (*doubleIndPtr == INVALID_PTR) {
    LGA_LOGGER_WARNING("[doubleIndGetPos] Allocating doubleInd");
    *doubleIndPtr = getFreeBlock();
    if (cleanBlock(*doubleIndPtr) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[doubleIndGetPos] Couldnt clean the doubleInd block");
      return FAILED;
    }
    setBitmap2(BLOCK_TYPE, *doubleIndPtr, BLOCK_BUSY);
    openDirectory.blocksFileSize += 1;
    return SINGLE_PTR + 1;
  }

  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0;
  int ptrPerBlock = (BLOCK_SIZE_BYTES/sizeof(DWORD));

  LGA_LOGGER_DEBUG("[doubleIndGetPos] Reading Block");
  if (readBlock(*doubleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[doubleIndGetPos] Couldnt read");
    return FAILED;
  }
  int i = 0;
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    LGA_LOGGER_DEBUG("[doubleIndGetPos] Getting Data");
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[doubleIndGetPos] Couldnt getData");
      return FAILED;
    }
    LGA_LOGGER_DEBUG("[doubleIndGetPos] Searching for position");
    if(*((DWORD*)ptrBuffer) != 0 && *((DWORD*)ptrBuffer) != INVALID_PTR) {

      try = singleIndGetPos(((DWORD*)ptrBuffer), newBlock) - (SECOND_REG + 1);
      if (try >= 0) try = try + (i * REGISTERS_PER_BLOCK * PTR_PER_BLOCK);

    } else {
      newNewBlock = getFreeBlock();
      if (cleanBlock(newNewBlock) != SUCCEEDED) {
        LGA_LOGGER_ERROR("[doubleIndGetPos] Couldnt clean the singleInd block");
        return FAILED;
      }
      setBitmap2(BLOCK_TYPE, newNewBlock, BLOCK_BUSY);
      changeWriteBlock(*doubleIndPtr, i*sizeof(DWORD), (char*)&newNewBlock, sizeof(DWORD));
      openDirectory.blocksFileSize += 1;
      openDirectory.bytesFileSize += sizeof(DWORD);
      try = (i * REGISTERS_PER_BLOCK * PTR_PER_BLOCK);
    }
    if (try >= 0) return try + (SINGLE_PTR + 1);
  }
  return BLOCK_FULL;
}

// OK
int doubleIndWrite(DWORD doubleIndPtr, int position, char * fileRecord) {

  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)], tempBlockBuffer[BLOCK_SIZE_BYTES];
  int ptrPerBlock = (BLOCK_SIZE_BYTES/sizeof(DWORD));
  int ptrPosition = floor((position/(REGISTERS_PER_BLOCK * PTR_PER_BLOCK)));
  int normalizedPtrPosition = position - (ptrPosition * REGISTERS_PER_BLOCK * PTR_PER_BLOCK);
  int newBlock;

  if (readBlock(doubleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[doubleIndWrite] Couldnt read");
    return FAILED;
  }
  if (getDataFromDisk(ptrBuffer, ptrPosition*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[doubleIndWrite] Couldnt getData");
    return FAILED;
  }

  // Se não existe o endereço do bloco desejado dentro do Single, cria ele
  if (*((DWORD*)ptrBuffer) == 0) {
    newBlock = getFreeBlock();
    if (cleanBlock(newBlock) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[doubleIndWrite] Couldnt clean the doubleInd block");
      return FAILED;
    }
    setBitmap2(BLOCK_TYPE,newBlock, BLOCK_BUSY);
    changeWriteBlock(doubleIndPtr, ptrPosition * sizeof(DWORD), (char*)&newBlock, sizeof(DWORD));
    openDirectory.blocksFileSize += 1;
    openDirectory.bytesFileSize += sizeof(DWORD);
    strcpy(ptrBuffer,(char*)&newBlock);
  }

  // Escreve o registro dentro do bloco que está dentro do bloco do single.
  if (singleIndWrite(*((DWORD*)ptrBuffer), normalizedPtrPosition, fileRecord) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[doubleIndWrite] Couldnt changeWrite");
    return FAILED;
  }
  LGA_LOGGER_LOG("[doubleIndWrite] Sucess");
  return SUCCEEDED;
}

int singleIndPrint(DWORD singleIndPtr) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0;

  if (singleIndPtr == INVALID_PTR) {
    return SUCCEEDED;
  }

  // Coloca o bloco de indireção no buffer
  if (readBlock(singleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[singleIndPrint] Couldnt read");
    return FAILED;
  }

  int i = 0;
  // Le cada ponteiro do bloco de semi indireção
  for(i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[singleIndPrint] Couldnt getData");
      return FAILED;
    }
    // Faz alguma coisa com o ponteiro de bloco (já está apontando para um bloco de dados aqui)
    if(*((DWORD*)ptrBuffer) != TYPEVAL_INVALIDO) {
      _printEntries(*((DWORD*)ptrBuffer));
    }
  }
  return SUCCEEDED;
}

int doubleIndPrint(DWORD doubleIndPtr) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0;
  int ptrPerBlock = (BLOCK_SIZE_BYTES/sizeof(DWORD));

  if (doubleIndPtr == INVALID_PTR) {
    return SUCCEEDED;
  }

  // Le o bloco de dupla indireção no buffer
  if (readBlock(doubleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[doubleIndPrint] Couldnt read");
    return FAILED;
  }
  int i = 0;
  // Le cada ponteiro de indireção do bloco
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[doubleIndPrint] Couldnt getData");
      return FAILED;
    }

    // Faz alguma coisa com o ponteiro de INDIREÇÃO (aqui não é bloco de dados ainda)
    // Por isso que sempre faço 2 funções, single e Double, por aqui posso chamar
    // a single e ela faz o resto.
    if(*((DWORD*)ptrBuffer) != TYPEVAL_INVALIDO) {
      singleIndPrint(*((DWORD*)ptrBuffer));
    }
  }
  return SUCCEEDED;
}

int singleIndInvalidate(DWORD singleIndPtr, int pos, int offset) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0;

  if (singleIndPtr == INVALID_PTR) {
    return SUCCEEDED;
  }
  // Coloca o bloco de indireção no buffer
  if (readBlock(singleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[singleIndPrint] Couldnt read");
    return FAILED;
  }

  int i = 0;
  // Le cada ponteiro do bloco de semi indireção
  for(i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[singleIndPrint] Couldnt getData");
      return FAILED;
    }
    // Faz alguma coisa com o ponteiro de bloco (já está apontando para um bloco de dados aqui)
    if(*((DWORD*)ptrBuffer) != INVALID_PTR && i + offset >= pos) {
      setBitmap2(BLOCK_TYPE, *((DWORD*)ptrBuffer), BLOCK_FREE);
      *((DWORD*)ptrBuffer) = TYPEVAL_INVALIDO;
    }
  }
  return SUCCEEDED;
}

int doubleIndInvalidate(DWORD doubleIndPtr, int pos) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0;
  int ptrPerBlock = (BLOCK_SIZE_BYTES/sizeof(DWORD));

  if (doubleIndPtr == INVALID_PTR) {
    return SUCCEEDED;
  }

  // Le o bloco de dupla indireção no buffer
  if (readBlock(doubleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[doubleIndPrint] Couldnt read");
    return FAILED;
  }
  int i = 0;
  // Le cada ponteiro de indireção do bloco
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[doubleIndPrint] Couldnt getData");
      return FAILED;
    }

    // Faz alguma coisa com o ponteiro de INDIREÇÃO (aqui não é bloco de dados ainda)
    // Por isso que sempre faço 2 funções, single e Double, por aqui posso chamar
    // a single e ela faz o resto.
    if(*((DWORD*)ptrBuffer) != TYPEVAL_INVALIDO && 2*BLOCK_SIZE_BYTES + PTR_PER_BLOCK * BLOCK_SIZE_BYTES + (i * PTR_PER_BLOCK * BLOCK_SIZE_BYTES) >= pos) {
      singleIndInvalidate(*((DWORD*)ptrBuffer), pos, floor((2*BLOCK_SIZE_BYTES + PTR_PER_BLOCK * BLOCK_SIZE_BYTES + (i * PTR_PER_BLOCK * BLOCK_SIZE_BYTES))/BLOCK_SIZE_BYTES));
      setBitmap2(BLOCK_TYPE, *((DWORD*)ptrBuffer), BLOCK_FREE);
    }
  }
  return SUCCEEDED;
}

void printBlock(DWORD blockPos) {
  char block[BLOCK_SIZE_BYTES];
  readBlock(blockPos, block, BLOCK_SIZE_BYTES);
  int i = 0;
  for (i = 0; i < BLOCK_SIZE_BYTES; i++) {
    printf("%d ",block[i] );
  }
  printf("\n");
}

int cleanBlock(DWORD blockPos) {
  char block[BLOCK_SIZE_BYTES];

  int i = 0;
  for ( i = 0; i < BLOCK_SIZE_BYTES; i++) {
    block[i] = 0;
  }

  if (changeWriteBlock(blockPos, 0, block, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[cleanBlock] Couldnt changeWriteBlock");
    return FAILED;
  }
  LGA_LOGGER_DEBUG("[cleanBlock] SUCCEEDED");
  return SUCCEEDED;
}

int _removeInode_SingleInd(DWORD singleIndPtr) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0;

  if (singleIndPtr == INVALID_PTR) {
    return SUCCEEDED;
  }
  if (readBlock(singleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_removeInode_SingleInd] Couldnt read");
    return FAILED;
  }
  int i = 0;
  for(  i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[_removeInode_SingleInd] Couldnt getData");
      return FAILED;
    }

    if(*((DWORD*)ptrBuffer) != TYPEVAL_INVALIDO) {
      if (setBitmap2(BLOCK_TYPE, *((DWORD*)ptrBuffer), BLOCK_FREE) != SUCCEEDED) {
        LGA_LOGGER_ERROR("[_removeInode_SingleInd] Couldnt set bitmap");
      }
    }
  }
  if (setBitmap2(BLOCK_TYPE, singleIndPtr, BLOCK_FREE) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_removeInode_SingleInd] Couldnt set bitmap");
  }
  return SUCCEEDED;
}

int _removeInode_DoubleInd(DWORD doubleIndPtr) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0;

  if (doubleIndPtr == INVALID_PTR) {
    return SUCCEEDED;
  }
  if (readBlock(doubleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_removeInode_DoubleInd] Couldnt read");
    return FAILED;
  }
  int i = 0;
  for(i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[_removeInode_DoubleInd] Couldnt getData");
      return FAILED;
    }

    if(*((DWORD*)ptrBuffer) != TYPEVAL_INVALIDO) {
      if (_removeInode_SingleInd(*((DWORD*)ptrBuffer)) != SUCCEEDED) {
        LGA_LOGGER_ERROR("[_removeInode_DoubleInd] Couldnt remove SingInd");
      }
      if (setBitmap2(BLOCK_TYPE, *((DWORD*)ptrBuffer), BLOCK_FREE) != SUCCEEDED) {
        LGA_LOGGER_ERROR("[_removeInode_DoubleInd] Couldnt set bitmap");
      }
    }
  }
  if (setBitmap2(BLOCK_TYPE, doubleIndPtr, BLOCK_FREE) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_removeInode_DoubleInd] Couldnt set bitmap");
  }
  return SUCCEEDED;
}

int removeFileRecord(DWORD inodePos, char* name, int *fileRecordPtr) {
  char inode[INODE_SIZE];
  int searchResult;
  getInode(inodePos, inode);

  if(((Inode*)inode)->dataPtr[0] != INVALID_PTR) {
    searchResult = removeFileRecord_Simple(((Inode*)&inode)->dataPtr[0], name);
    if (searchResult != NOT_FOUND) {
      if (searchResult == FOUND) {
        *fileRecordPtr = 0;
        return SUCCEEDED;
      } else {
        LGA_LOGGER_ERROR("[removeFileRecord] Couldnt remove the first entry");
        return FAILED;
      }
    }
  }
  if(((Inode*)inode)->dataPtr[1] != INVALID_PTR) {
    searchResult = removeFileRecord_Simple(((Inode*)inode)->dataPtr[1], name);
    if (searchResult != NOT_FOUND) {
      if (searchResult == FOUND) {
        *fileRecordPtr = 1;
        return SUCCEEDED;
      } else {
        LGA_LOGGER_ERROR("[removeFileRecord] Couldnt remove the second entry");
        return FAILED;
      }
    }
  }

  if(((Inode*)inode)->singleIndPtr != INVALID_PTR) {
    searchResult = _removeFileRecord_SingleInd(((Inode*)inode)->singleIndPtr, name);
    if (searchResult != NOT_FOUND) {
      if (searchResult == FOUND) {
        *fileRecordPtr = 2;
        return SUCCEEDED;
      } else {
        LGA_LOGGER_ERROR("[removeFileRecord] Couldnt remove the singleIndPtr entry");
        return FAILED;
      }
    }
  }

  if(((Inode*)inode)->doubleIndPtr != INVALID_PTR) {
    searchResult = _removeFileRecord_DoubleInd(((Inode*)inode)->doubleIndPtr, name);
    if (searchResult != NOT_FOUND) {
      if (searchResult == FOUND) {
        *fileRecordPtr = 3;
        return SUCCEEDED;
      } else {
        LGA_LOGGER_ERROR("[removeFileRecord] Couldnt remove the doubleIndPtr entry");
        return FAILED;
      }
    }
  }
  *fileRecordPtr = FAILED;
  return NOT_FOUND;
}

int removeFileRecord_Simple(DWORD ptr, char* name) {
  char diskBuffer[BLOCK_SIZE_BYTES], registerBuffer[REGISTER_SIZE];

  //Le o bloco apontado por ptr e coloca no buffer
  if (readBlock(ptr, diskBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[removeFileRecord_Simple] couldnt read the entry");
    return FAILED;
  }
  int i ;
  //Le de registro em registro dentro do buffer
  for (i = 0; i < REGISTERS_PER_BLOCK; i++) {
    if (getRegisterFile(i, diskBuffer, BLOCK_SIZE_BYTES, registerBuffer) !=SUCCEEDED) {
      LGA_LOGGER_ERROR("[removeFileRecord_Simple] couldnt get the register file");
      return FAILED;
    }

    //Pode ter sido deletada uma entrada do diretorio e setado para INVALIDO entao nao consideramos esse nome desse residuo lixo
    if(((FileRecord*) registerBuffer)->TypeVal != TYPEVAL_INVALIDO)
    {
      //Se registro possuir o nome passado é porque é o arquivo procurado
      if (strcmp(((FileRecord*) registerBuffer)->name, name) == SUCCEEDED) {
        ((FileRecord*) registerBuffer)->TypeVal = TYPEVAL_INVALIDO;
        if(changeWriteBlock(ptr, i * REGISTER_SIZE, registerBuffer, REGISTER_SIZE)!= SUCCEEDED){
      		LGA_LOGGER_ERROR("[removeFileRecord_Simple] Couldnt changeWriteBlock");
      		return FAILED;
      	}
        openDirectory.bytesFileSize -= REGISTER_SIZE;
        return FOUND;
      }
    }
  }
  return NOT_FOUND;
}

int _removeFileRecord_SingleInd(DWORD singleIndPtr, char* name) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0, searchResult;

  if (readBlock(singleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_removeInode_SingleInd] Couldnt read");
    return FAILED;
  }
  int i = 0;
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[_removeInode_SingleInd] Couldnt getData");
      return FAILED;
    }

    if(*((DWORD*)ptrBuffer) != INVALID_PTR && *((DWORD*)ptrBuffer) != 0) {
      searchResult = removeFileRecord_Simple(*((DWORD*)ptrBuffer), name);
      if (searchResult != NOT_FOUND) {
        if (searchResult == FOUND) {
          return FOUND;
        } else {
          LGA_LOGGER_ERROR("[removeFileRecord] Couldnt remove the doubleIndPtr entry");
          return FAILED;
        }
      }
    }
  }
  return NOT_FOUND;
}

int _removeFileRecord_DoubleInd(DWORD doubleIndPtr, char* name) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0, searchResult;

  if (readBlock(doubleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_removeFileRecord_DoubleInd] Couldnt read");
    return FAILED;
  }
  int i = 0;
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[_removeFileRecord_DoubleInd] Couldnt getData");
      return FAILED;
    }

    if(*((DWORD*)ptrBuffer) != INVALID_PTR && *((DWORD*)ptrBuffer) != 0) {
      searchResult = _removeFileRecord_SingleInd(*((DWORD*)ptrBuffer), name);
      if (searchResult != NOT_FOUND) {
        if (searchResult == FOUND) {
          return FOUND;
        } else {
          LGA_LOGGER_ERROR("[_removeFileRecord_DoubleInd] Couldnt remove the doubleIndPtr entry");
          return FAILED;
        }
      }
    }
  }
  return NOT_FOUND;
}

int isEmptyFileRecord(DWORD ptr) {
  char diskBuffer[BLOCK_SIZE_BYTES], registerBuffer[REGISTER_SIZE];

  //Le o bloco apontado por ptr e coloca no buffer
  if (readBlock(ptr, diskBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[removeFileRecord_Simple] couldnt read the entry");
    return FAILED;
  }
  int i = 0;
  //Le de registro em registro dentro do buffer
  for ( i = 0; i < REGISTERS_PER_BLOCK; i++) {
    if (getRegisterFile(i, diskBuffer, BLOCK_SIZE_BYTES, registerBuffer) !=SUCCEEDED) {
      LGA_LOGGER_ERROR("[removeFileRecord_Simple] couldnt get the register file");
      return FAILED;
    }

    //Pode ter sido deletada uma entrada do diretorio e setado para INVALIDO entao nao consideramos esse nome desse residuo lixo
    if(((FileRecord*) registerBuffer)->TypeVal != TYPEVAL_INVALIDO) {
      return C_FALSE;
    }
  }
  return C_TRUE;
}

int isEmptyPtr(DWORD ptr) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, newNewBlock = 0, searchResult;

  if (readBlock(ptr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_removeInode_SingleInd] Couldnt read");
    return FAILED;
  }
  int i = 0;
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[_removeInode_SingleInd] Couldnt getData");
      return FAILED;
    }

    if(*((DWORD*)ptrBuffer) != TYPEVAL_INVALIDO) {
      return C_FALSE;
    }
  }
  return C_TRUE;
}

int garbageCollector(DWORD inodePos, int fileRecordPtr) {
 char inode[INODE_SIZE];
 int result = 0;
 getInode(inodePos, inode);

 if (((Inode*)inode)->dataPtr[1] != INVALID_PTR && fileRecordPtr == 0) {
   result = isEmptyFileRecord(((Inode*)inode)->dataPtr[0]);
   if (result == C_TRUE) {
     ((Inode*)inode)->dataPtr[0] = INVALID_PTR;
    openDirectory.blocksFileSize -= 1;

     return SUCCEEDED;
   } else if (result == FAILED) {
     LGA_LOGGER_ERROR("[garbageCollector] error");
     return FAILED;
   }
 } else if (((Inode*)inode)->dataPtr[1] != INVALID_PTR && fileRecordPtr == 1) {
   result = isEmptyFileRecord(((Inode*)inode)->dataPtr[1]);
   if (result == C_TRUE) {
     setBitmap2(BLOCK_TYPE,((Inode*)inode)->dataPtr[1],BLOCK_FREE);
     ((Inode*)inode)->dataPtr[1] = INVALID_PTR;
    openDirectory.blocksFileSize -= 1;
     return SUCCEEDED;

   } else if (result == FAILED) {
     LGA_LOGGER_ERROR("[garbageCollector] error");
     return FAILED;
   }
 } else if (((Inode*)inode)->singleIndPtr != INVALID_PTR && fileRecordPtr == 2) {
   result = _isEmptyFile_SingleInd(((Inode*)inode)->singleIndPtr);
   if (result == C_TRUE) {
     setBitmap2(BLOCK_TYPE,((Inode*)inode)->singleIndPtr,BLOCK_FREE);
     ((Inode*)inode)->singleIndPtr = INVALID_PTR;
    openDirectory.blocksFileSize -= 1;
     return SUCCEEDED;

   } else if (result == FAILED) {
     LGA_LOGGER_ERROR("[garbageCollector] error");
     return FAILED;
   }
 } else if (((Inode*)inode)->doubleIndPtr != INVALID_PTR && fileRecordPtr == 3) {
   result = _isEmptyFile_DoubleInd(((Inode*)inode)->doubleIndPtr);
   if (result == C_TRUE) {
     setBitmap2(BLOCK_TYPE,((Inode*)inode)->doubleIndPtr,BLOCK_FREE);
     ((Inode*)inode)->doubleIndPtr = INVALID_PTR;
    openDirectory.blocksFileSize -= 1;
     return SUCCEEDED;

   } else if (result == FAILED) {
     LGA_LOGGER_ERROR("[garbageCollector] error");
     return FAILED;
   }
 }
 return SUCCEEDED;
}

int _isEmptyFile_SingleInd(DWORD singleIndPtr) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, result = 0;
  DWORD zero = INVALID_PTR;

  // Coloca o bloco de indireção no buffer
  if (readBlock(singleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_isEmptyFile_SingleInd] Couldnt read");
    return FAILED;
  }
  int i = 0;
  // Le cada ponteiro do bloco de semi indireção
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[_isEmptyFile_SingleInd] Couldnt getData");
      return FAILED;
    }
    // Faz alguma coisa com o ponteiro de bloco (já está apontando para um bloco de dados aqui)
    if(*((DWORD*)ptrBuffer) != INVALID_PTR && *((DWORD*)ptrBuffer) != 0 ) {
      result = isEmptyFileRecord(*((DWORD*)ptrBuffer));
      if (result == C_TRUE) {
        if (changeWriteBlock(singleIndPtr, i * sizeof(DWORD), (char*)&zero, sizeof(DWORD)) != SUCCEEDED) {
          LGA_LOGGER_ERROR("[_isEmptyFile_SingleInd] Couldnt changeWriteBlock");
          return FAILED;
        }
        setBitmap2(BLOCK_TYPE,*((DWORD*)ptrBuffer),BLOCK_FREE);
    openDirectory.blocksFileSize -= 1;
    openDirectory.bytesFileSize -= sizeof(DWORD);
      } else if (result == FAILED) {
        LGA_LOGGER_ERROR("[_isEmptyFile_SingleInd] Couldnt isEmptyFileRecord");
        return FAILED;
      } else {
        try = 1;
      }
    }
  }
  if (try == 0) {
    return C_TRUE;
  }
  return C_FALSE;
}

int _isEmptyFile_DoubleInd(DWORD doubleIndPtr) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int try = 0, result = 0;
  DWORD zero = INVALID_PTR;

  // Coloca o bloco de indireção no buffer
  if (readBlock(doubleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[_isEmptyFile_DoubleInd] Couldnt read");
    return FAILED;
  }
  int i = 0;
  // Le cada ponteiro do bloco de semi indireção
  for( i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    if (getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[_isEmptyFile_DoubleInd] Couldnt getData");
      return FAILED;
    }
    // Faz alguma coisa com o ponteiro de bloco (já está apontando para um bloco de dados aqui)
    if(*((DWORD*)ptrBuffer) != INVALID_PTR && *((DWORD*)ptrBuffer) != 0 ) {
      result = _isEmptyFile_SingleInd(*((DWORD*)ptrBuffer));
      if (result == C_TRUE) {
        if (changeWriteBlock(doubleIndPtr, i * sizeof(DWORD), (char*)&zero, sizeof(DWORD)) != SUCCEEDED) {
          LGA_LOGGER_ERROR("[_isEmptyFile_DoubleInd] Couldnt changeWriteBlock");
          return FAILED;
        }
        setBitmap2(BLOCK_TYPE,*((DWORD*)ptrBuffer),BLOCK_FREE);
        openDirectory.blocksFileSize -= 1;
        openDirectory.bytesFileSize -= sizeof(DWORD);
      } else if (result == FAILED) {
        LGA_LOGGER_ERROR("[_isEmptyFile_DoubleInd] Couldnt isEmptyFileRecord");
        return FAILED;
      } else {
        try = 1;
      }
    }
  }
  if (try == 0) {
    return C_TRUE;
  }
  return C_FALSE;
}

void printBitmap(int BITMAP_TYPE, int MAXSIZE, int isInode ){
  int  i = 0;
  char tipo[6];
  (isInode == 1) ? strcpy(tipo,"inode") : strcpy(tipo,"bloco");

  for (i = 0; i < MAXSIZE; i++) {
    printf("%s: %d   %d= ",tipo,i,getBitmap2(BITMAP_TYPE, i) );
  }
}

void printQuantInode() {
  int maxInodes = superBlock.freeInodeBitmapSize * BLOCK_SIZE_BYTES;
  int iFree=0, iBusy=0, error=0;
    int i = 0;
  for(i = 0; i < maxInodes; i++) {
    if (getBitmap2(INODE_TYPE, i) == INODE_FREE) {
      iFree++;
    } else if (getBitmap2(INODE_TYPE, i) == INODE_BUSY) {
      iBusy++;
    } else {
      error++;
    }
  }
  printf("iNodes Livres: %d   iNodes Ocupados: %d  Error: %d\n", iFree, iBusy, error);
}

void printQuantBlock() {
  int maxBlocks = superBlock.freeBlocksBitmapSize * BLOCK_SIZE_BYTES;
  int blockFree=0, blockBusy=0, error=0;
  int i = 0;
  for(i = 0; i < maxBlocks; i++) {
    if (getBitmap2(BLOCK_TYPE, i) == BLOCK_FREE) {
      blockFree++;
    } else if (getBitmap2(BLOCK_TYPE, i) == BLOCK_BUSY) {
      blockBusy++;
    } else {
      error++;
    }
  }
  printf("Blocos Livres: %d   Blocos Ocupados: %d  Error: %d\n", blockFree, blockBusy,error);
}

int writeOnPtr(DWORD ptr, int * CP, char * content, int size){
  int CPPosBlock = floor((*CP / BLOCK_SIZE_BYTES));
  int CPPos = *CP - CPPosBlock * BLOCK_SIZE_BYTES;
  char blockBuffer[BLOCK_SIZE_BYTES];
  int i, cpBuf = 0;


  if(readBlock(ptr, blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED){
    return FAILED;
  }

  for(i = 0; i < BLOCK_SIZE_BYTES; i++){
    if(i >= CPPos && i < CPPos + size){
      blockBuffer[i] = content[i - CPPos];
      cpBuf ++;
    }
  }
  if(writeBlock(ptr, (char *)&blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED){
    return FAILED;
  }

  for(i = 0; i < BLOCK_SIZE_BYTES; i++){
  }

  *CP += cpBuf;
  return SUCCEEDED;
}

int writeOnIndirection_single(DWORD ptr, int * CP, char * content, int size){
  char indirectionBlock[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int i, j;
  if(readBlock(ptr, indirectionBlock, BLOCK_SIZE_BYTES) != SUCCEEDED){
    return FAILED;
  }

  for(i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++){
    if(*CP < size){
      if(getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), indirectionBlock, BLOCK_SIZE_BYTES) != SUCCEEDED){
        return FAILED;
      }
      if(*((DWORD*)ptrBuffer) == INVALID_PTR){
        *((DWORD*)ptrBuffer) = getFreeBlock();
        if(*((DWORD*)ptrBuffer) < SUCCEEDED){
          LGA_LOGGER_ERROR("[writeOnIndirection_single] Couldn't allocate block");
          return FAILED;
        }
        setBitmap2(BLOCK_TYPE, *((DWORD*)ptrBuffer), BLOCK_BUSY);
        for(j = 0; j < sizeof(DWORD); j++){
          indirectionBlock[j + i*sizeof(DWORD)] = ptrBuffer[j];
        }
        writeBlock(ptr, indirectionBlock, BLOCK_SIZE_BYTES);
      }
      writeOnPtr(*((DWORD*)ptrBuffer), CP, content, size);
    }
    else{
      return SUCCEEDED;
    }
  }
  return SUCCEEDED;
}

int writeOnIndirection_double(DWORD ptr, int * CP, char * content, int size){
  char indirectionBlock[BLOCK_SIZE_BYTES], ptrBuffer[BLOCK_SIZE_BYTES];
  int i;
  if(readBlock(ptr, indirectionBlock, BLOCK_SIZE_BYTES) != SUCCEEDED){
    return FAILED;
  }

  for(i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++){
    if(*CP < size){
      if(getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), indirectionBlock, BLOCK_SIZE_BYTES) != SUCCEEDED){
        return FAILED;
      }
      if(*((DWORD*)ptrBuffer) == INVALID_PTR){
        *((DWORD*)ptrBuffer) = getFreeBlock();
        if(*((DWORD*)ptrBuffer) < SUCCEEDED){
          LGA_LOGGER_ERROR("[writeOnIndirection_double] Couldn't allocate block");
          return FAILED;
        }
        setBitmap2(BLOCK_TYPE, *((DWORD*)ptrBuffer), BLOCK_BUSY);
      }
      writeOnIndirection_single(*((DWORD*)ptrBuffer), CP, content, size);
    }
    else{
      return SUCCEEDED;
    }
  }

  return BLOCK_FULL;
}

int writeContentOnDisk(Inode * fileInode, int *CP, char * content, int size){
  int initialCP = *CP;
  int written = 0;
  while(*CP - initialCP < size){
    LGA_LOGGER_DEBUG("[getByteBlockPositioning] Writing on dataptr[0]");
    //byte In dataPtr[0]
    if(*CP < BLOCK_SIZE_BYTES){
      if(fileInode->dataPtr[0] == INVALID_PTR){
        fileInode->dataPtr[0] = getFreeBlock();
        if(fileInode->dataPtr[0] < SUCCEEDED){
          LGA_LOGGER_ERROR("[writeContentOnDisk] Couldn't allocate block for ptr[0]");
          return FAILED;
        }
        LGA_LOGGER_DEBUG("[writeContentOnDisk] allocated block for ptr[0]");
        setBitmap2(BLOCK_TYPE, fileInode->dataPtr[0], BLOCK_BUSY);
      }
      if(writeOnPtr(fileInode->dataPtr[0], CP, content, size) != SUCCEEDED){
        LGA_LOGGER_ERROR("[writeContentOnDisk] failed to write on ptr[0]");
        return FAILED;
      }
      LGA_LOGGER_DEBUG("[writeContentOnDisk] written on ptr[0]");
    }else if(*CP < 2 * BLOCK_SIZE_BYTES){
      LGA_LOGGER_DEBUG("[getByteBlockPositioning] Writing on dataptr[1]");
      //byte In dataPtr[1]
      if(fileInode->dataPtr[1] == INVALID_PTR){
        fileInode->dataPtr[1] = getFreeBlock();
        if(fileInode->dataPtr[1] < SUCCEEDED){
          LGA_LOGGER_ERROR("[writeContentOnDisk] Couldn't allocate block for ptr[1]");
          return FAILED;
        }
        LGA_LOGGER_DEBUG("[writeContentOnDisk] allocated block for ptr[1]");
        setBitmap2(BLOCK_TYPE, fileInode->dataPtr[1], BLOCK_BUSY);
      }
      if(writeOnPtr(fileInode->dataPtr[1], CP, content, size) != SUCCEEDED){
        LGA_LOGGER_ERROR("[writeContentOnDisk] failed to write on ptr[1]");
        return FAILED;
      }
    }else if(*CP < BLOCK_SIZE_BYTES * PTR_PER_BLOCK + 2 * BLOCK_SIZE_BYTES){
      LGA_LOGGER_DEBUG("[getByteBlockPositioning] Writing on SingleInd");
      //byte on singleInd
      if(fileInode->singleIndPtr == INVALID_PTR){
        fileInode->singleIndPtr = getFreeBlock();
        if(fileInode->singleIndPtr < SUCCEEDED){
          LGA_LOGGER_ERROR("[writeContentOnDisk] Couldn't allocate block for single");
          return FAILED;
        }
        LGA_LOGGER_DEBUG("[writeContentOnDisk] allocated block for single");
        setBitmap2(BLOCK_TYPE, fileInode->singleIndPtr, BLOCK_BUSY);
        cleanIndBlock(fileInode->singleIndPtr);
      }
      if(writeOnIndirection_single(fileInode->singleIndPtr, CP, content, size) != SUCCEEDED){
        LGA_LOGGER_ERROR("[writeContentOnDisk] failed to write on single indirection");
        return FAILED;
      }
    }else{
      //byte on doubleInd
      LGA_LOGGER_DEBUG("[getByteBlockPositioning] Writing on DoubleInd");
      if(*CP < BLOCK_SIZE_BYTES * PTR_PER_BLOCK + 2 * BLOCK_SIZE_BYTES){
        if(fileInode->doubleIndPtr == INVALID_PTR){
          fileInode->doubleIndPtr = getFreeBlock();
          if(fileInode->doubleIndPtr < SUCCEEDED){
            LGA_LOGGER_ERROR("[writeContentOnDisk] Couldn't allocate block for double");
            return FAILED;
          }
          LGA_LOGGER_DEBUG("[writeContentOnDisk] allocated block for double");
          setBitmap2(BLOCK_TYPE, fileInode->doubleIndPtr, BLOCK_BUSY);
        }
        int doubleResult = writeOnIndirection_double(fileInode->doubleIndPtr, CP, content, size);
        if(doubleResult != SUCCEEDED){
          if(doubleResult == BLOCK_FULL){
            break;
          }else{
            LGA_LOGGER_ERROR("[writeContentOnDisk] failed to write on double indirection");
            return FAILED;
          }
        }
      }
    }
  }
  return *CP - initialCP;
}

int readOnPtr(DWORD ptr, int * CP, char * content, int size){
  int CPPosBlock = floor((*CP / BLOCK_SIZE_BYTES));
  int CPPos = *CP - CPPosBlock * BLOCK_SIZE_BYTES;
  char blockBuffer[BLOCK_SIZE_BYTES];
  int i, cpBuf = 0;


  if(readBlock(ptr, blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED){
    return FAILED;
  }

  for(i = 0; i < BLOCK_SIZE_BYTES; i++){
    if(i >= CPPos && i < CPPos + size){
      content[CPPosBlock * BLOCK_SIZE_BYTES + i - CPPos] = blockBuffer[i];
      cpBuf ++;
    }
  }

  *CP += cpBuf;
  return SUCCEEDED;
}

int readOnIndirection_single(DWORD ptr, int * CP, char * content, int size){
  char indirectionBlock[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)];
  int i;


  if(readBlock(ptr, indirectionBlock, BLOCK_SIZE_BYTES) != SUCCEEDED){
    return FAILED;
  }
  for(i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++){
    if(*CP < size){
      if(getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), indirectionBlock, BLOCK_SIZE_BYTES) != SUCCEEDED){
        return FAILED;
      }
      if(*((DWORD*)ptrBuffer) == INVALID_PTR){
        return BLOCK_FULL;
      }
      readOnPtr(*((DWORD*)ptrBuffer), CP, content, size);

    }
    else{

      return SUCCEEDED;
    }
  }
  return SUCCEEDED;
}

int readOnIndirection_double(DWORD ptr, int * CP, char * content, int size){
  char indirectionBlock[BLOCK_SIZE_BYTES], ptrBuffer[BLOCK_SIZE_BYTES];
  int i;
  if(readBlock(ptr, indirectionBlock, BLOCK_SIZE_BYTES) != SUCCEEDED){
    return FAILED;
  }

  for(i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++){
    if(*CP < size){
      if(getDataFromDisk(ptrBuffer, i*sizeof(DWORD), sizeof(DWORD), indirectionBlock, BLOCK_SIZE_BYTES) != SUCCEEDED){
        return FAILED;
      }
      if(*((DWORD*)ptrBuffer) == INVALID_PTR){
        return SUCCEEDED;
      }
      readOnIndirection_single(*((DWORD*)ptrBuffer), CP, content, size);
    }
    else{
      return SUCCEEDED;
    }
  }

  return BLOCK_FULL;
}

int readContentOnDisk(Inode * fileInode, int *CP, char * content, int size){
  int initialCP = *CP;
  int written = 0;
  if(fileInode->bytesFileSize < size){
    size = fileInode->bytesFileSize;
  }
  while(*CP - initialCP < size){

    //byte In dataPtr[0]
    if(*CP < BLOCK_SIZE_BYTES){
      LGA_LOGGER_DEBUG("[readContentOnDisk] reading on dataptr[0]");
      if(fileInode->dataPtr[0] == INVALID_PTR){
        break;
        }
      if(readOnPtr(fileInode->dataPtr[0], CP, content, size) != SUCCEEDED){
        LGA_LOGGER_ERROR("[readContentOnDisk] failed to read on ptr[0]");
        return FAILED;
      }
      LGA_LOGGER_DEBUG("[readContentOnDisk] Read ptr[0]");
    }else if(*CP < 2 * BLOCK_SIZE_BYTES){
      LGA_LOGGER_DEBUG("[readContentOnDisk] Reading on dataptr[1]");
      //byte In dataPtr[1]
      if(fileInode->dataPtr[1] == INVALID_PTR){
        break;
      }
      if(readOnPtr(fileInode->dataPtr[1], CP, content, size) != SUCCEEDED){
        LGA_LOGGER_ERROR("[readContentOnDisk] failed to read on ptr[1]");
        return FAILED;
      }
      LGA_LOGGER_DEBUG("[readContentOnDisk] Read ptr[1]");
    }else if(*CP < BLOCK_SIZE_BYTES * PTR_PER_BLOCK + 2 * BLOCK_SIZE_BYTES){
      LGA_LOGGER_DEBUG("[readContentOnDisk] reading on SingleInd");
      //byte on singleInd
      if(fileInode->singleIndPtr == INVALID_PTR){
        break;
      }
      int read = readOnIndirection_single(fileInode->singleIndPtr, CP, content, size);
      if(read != SUCCEEDED){
        if(read == BLOCK_FULL){
          break;
        }
        LGA_LOGGER_ERROR("[readContentOnDisk] failed to read on single indirection");
        return FAILED;
      }
      LGA_LOGGER_DEBUG("[readContentOnDisk] Read on single indirection");
    }else{
      //byte on doubleInd
      LGA_LOGGER_DEBUG("[readContentOnDisk] reading on DoubleInd");
      if(*CP < BLOCK_SIZE_BYTES * PTR_PER_BLOCK + 2 * BLOCK_SIZE_BYTES){
        if(fileInode->doubleIndPtr == INVALID_PTR){
          break;
        }
        int doubleResult = readOnIndirection_double(fileInode->doubleIndPtr, CP, content, size);
        if(doubleResult != SUCCEEDED){
          if(doubleResult == BLOCK_FULL){
            break;
          }else{
            return FAILED;
          }
        }
      }
    }
  }
  return *CP - initialCP;
}

int cleanIndBlock(DWORD ptr){
  DWORD block[BLOCK_SIZE_BYTES/sizeof(DWORD)];
  int i = 0;
  for (i = 0; i < BLOCK_SIZE_BYTES/sizeof(DWORD); i++) {
    block[i] = INVALID_PTR;
  }

  if (writeBlock(ptr, (char *)block, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[cleanIndBlock] Couldnt changeWriteBlock");
    return FAILED;
  }
  LGA_LOGGER_DEBUG("[cleanIndBlock] SUCCEEDED");
  return SUCCEEDED;
}

int getSpecificEntry_SingleInd(DWORD singleIndPtr, int entryNum, char* buffer) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)], tempBlockBuffer[BLOCK_SIZE_BYTES];
  int ptrPosition = floor((entryNum/REGISTERS_PER_BLOCK));
  int normalizedPtrPosition = entryNum - (ptrPosition * REGISTERS_PER_BLOCK);

  if (readBlock(singleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getSpecificEntry_SingleInd] Couldnt read");
    return FAILED;
  }

  if (getDataFromDisk(ptrBuffer, ptrPosition*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getSpecificEntry_SingleInd] Couldnt getData");
    return FAILED;
  }

  // Se não existe o endereço do bloco desejado dentro do Single, cria ele
  if (*((DWORD*)ptrBuffer) == INVALID_PTR) {
      LGA_LOGGER_ERROR("[getSpecificEntry_SingleInd] InvalidPTR");
      return FAILED;
  }

  if (readBlock(*((DWORD*)ptrBuffer),tempBlockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getSpecificEntry_SingleInd] Couldnt read temp");
    return FAILED;
  }

  if (getRegisterFile(normalizedPtrPosition, tempBlockBuffer, BLOCK_SIZE_BYTES, buffer) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getSpecificEntry_SingleInd] Couldnt changeWrite");
    return FAILED;
  }
  if (((FileRecord*)buffer)->TypeVal == TYPEVAL_INVALIDO) {
    LGA_LOGGER_WARNING("[getSpecificEntry_SingleInd] Record Invalid");
    return FAILED;
  }
  LGA_LOGGER_LOG("[getSpecificEntry_SingleInd] Sucess");
  return SUCCEEDED;
}

int getSpecificEntry_DoubleInd(DWORD doubleIndPtr, int entryNum, char* buffer) {
  char blockBuffer[BLOCK_SIZE_BYTES], ptrBuffer[sizeof(DWORD)], tempBlockBuffer[BLOCK_SIZE_BYTES];
  int ptrPosition = floor((entryNum/(REGISTERS_PER_BLOCK * PTR_PER_BLOCK)));
  int normalizedPtrPosition = entryNum - (ptrPosition * REGISTERS_PER_BLOCK * PTR_PER_BLOCK);

  if (readBlock(doubleIndPtr,blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getSpecificEntry_DoubleInd] Couldnt read");
    return FAILED;
  }

  if (getDataFromDisk(ptrBuffer, ptrPosition*sizeof(DWORD), sizeof(DWORD), blockBuffer, BLOCK_SIZE_BYTES) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getSpecificEntry_DoubleInd] Couldnt getData");
    return FAILED;
  }

  // Se não existe o endereço do bloco desejado dentro do Single, cria ele
  if (*((DWORD*)ptrBuffer) == INVALID_PTR) {
      LGA_LOGGER_ERROR("[getSpecificEntry_DoubleInd] InvalidPTR");
      return FAILED;
  }

  if (getSpecificEntry_SingleInd(*((DWORD*)ptrBuffer),normalizedPtrPosition, buffer) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[getSpecificEntry_DoubleInd] getSpecificEntry_SingleInd");
    return FAILED;
  }
  return SUCCEEDED;
}
