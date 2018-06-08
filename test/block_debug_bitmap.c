#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

  initializeSuperBlock();
  printAllEntries(openDirectory);
  printBitmap(BLOCK_TYPE, 100, 1 );
  mkdir2("Diretorio1");
  printAllEntries(openDirectory);
  printBitmap(BLOCK_TYPE, 100, 1 );
  rmdir2("/Diretorio1");
  printAllEntries(openDirectory);
  printBitmap(BLOCK_TYPE, 100, 1 );

}
