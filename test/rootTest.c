#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/bitmap2.h"
#include "../include/LGA_support.h"
#include "../include/LGA_logger.h"

int inodeWritingTest();
int blockWritingTest();

int main(){
  char buffer[INODE_SIZE],  registerF[REGISTER_SIZE], blockBuffer[4 * SECTOR_SIZE];
  initializeSuperBlock();
  getRootInode(buffer);

  readBlock(((Inode*)buffer)->dataPtr[0], blockBuffer, 4 * SECTOR_SIZE);

  getRegisterFile(1, blockBuffer, 4 * SECTOR_SIZE, registerF);

  if(strcmp(((FileRecord*)registerF)->name,"..") == 0) {
    LGA_LOGGER_TEST("ROOT SUCCEEDED");
  }
  else
    LGA_LOGGER_TEST("ROOT FAILED");

  return 0;
}
