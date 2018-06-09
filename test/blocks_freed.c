#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/t2fs.h"
#include "../include/LGA_support.h"
#include "../include/LGA_logger.h"

int main(){
  // mkdir2("debug");
  // chdir2("debug");
  /*chdir2("debug");
  create2("C");
  create2("CC");
  create2("CCCC");
  create2("CCCCC");
  create2("CCCCCC");
  create2("CCCCCCC");
  create2("CCCCCCCCC");

  printf("openDirectory ptr[0] = %d\n",openDirectory.dataPtr[0] );
  printf("openDirectory ptr[0] = %d\n",openDirectory.dataPtr[1] );
  printAllEntries(openDirectory);*/
  initializeSuperBlock();
  printAllEntries(openDirectory);
  LGA_LOGGER_TEST("Bitmap de block:\n");
  printBitmap(BLOCK_TYPE, 100, 0 );
  LGA_LOGGER_TEST("Fim Bitmap de block inicial:\n");
  mkdir2("debug");
  LGA_LOGGER_TEST("debug/ Bitmap de block:\n");
  printBitmap(BLOCK_TYPE, 100, 0 );
  LGA_LOGGER_TEST("Fim debug/Bitmap de block inicial:\n");
  chdir2("debug");
  LGA_LOGGER_TEST("No diretorio debug");
  printAllEntries(openDirectory);
  close2(create2("A"));
  close2(create2("AA"));
  close2(create2("AAAA"));
  close2(create2("AAAAA"));
  close2(create2("AAAAAA"));
  close2(create2("AAAAAAA"));
  close2(create2("AAAAAAAAA"));
  close2(create2("B"));
  close2(create2("BB"));
  close2(create2("BBBB"));
  close2(create2("BBBBB"));
  close2(create2("BBBBBB"));
  close2(create2("BBBBBBB"));
  close2(create2("BBBBBBBBB"));
  printf("ptr[0] = %d\n",openDirectory.dataPtr[0] );
  printf("ptr[1] = %d\n",openDirectory.dataPtr[1] );
  printf("Vou creatar os C\n\n\n\n\n\n\n\n");
  close2(create2("C"));
  close2(create2("CC"));
  close2(create2("CCCC"));
  close2(create2("CCCCC"));
  close2(create2("CCCCCC"));
  close2(create2("CCCCCCC"));
  close2(create2("CCCCCCCCC"));
  LGA_LOGGER_TEST("No diretorio debug");
  printAllEntries(openDirectory);

}
