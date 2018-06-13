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
  LGA_LOGGER_TEST("Fim Bitmap de block inicial\n");
  mkdir2("debug");
  LGA_LOGGER_TEST("Printando mkdir debug Bitmap de block:\n");
  printBitmap(BLOCK_TYPE, 100, 0 );
  LGA_LOGGER_TEST("Fim print mkdir debug Bitmap de block \n");
  chdir2("debug");
  LGA_LOGGER_TEST("No diretorio debug");
  printAllEntries(openDirectory);
  chdir2("./..");
  LGA_LOGGER_TEST("No diretorio /");
  printAllEntries(openDirectory);

  close2(create2("/debug/A"));
  close2(create2("/debug/AA"));
  close2(create2("/debug/AAAA"));
  close2(create2("/debug/AAAAA"));
  close2(create2("/debug/AAAAAA"));
  close2(create2("/debug/AAAAAAA"));
  close2(create2("/debug/AAAAAAAAA"));
  close2(create2("debug/B"));
  close2(create2("debug/BB"));
  close2(create2("debug/BBBB"));
  close2(create2("debug/BBBBB"));
  close2(create2("./debug/BBBBBB"));
  close2(create2("./debug/BBBBBBB"));
  close2(create2("./debug/BBBBBBBBB"));
  close2(create2("./debug/C"));
  close2(create2("./debug/CC"));
  close2(create2("./debug/CCCC"));
  close2(create2("./debug/CCCCC"));
  close2(create2("./debug/CCCCCC"));
  close2(create2("./debug/CCCCCCC"));
  close2(create2("./debug/CCCCCCCCC"));

  close2(create2("/debug/xAAAA"));
  close2(create2("/debug/xAAAAA"));
  close2(create2("/debug/xAAAAAA"));
  close2(create2("/debug/xAAAAAAA"));
  close2(create2("/debug/xAAAAAAAAA"));
  close2(create2("debug/xB"));
  close2(create2("debug/xBB"));
  close2(create2("debug/xBBBB"));
  close2(create2("debug/xBBBBB"));
  close2(create2("./debug/xBBBBBB"));
  close2(create2("./debug/xBBBBBBB"));
  close2(create2("./debug/xBBBBBBBBB"));
  close2(create2("./debug/xC"));
  close2(create2("./debug/xCC"));
  close2(create2("./debug/xCCCC"));
  close2(create2("./debug/xCCCCC"));
  close2(create2("./debug/xCCCCCC"));
  close2(create2("./debug/xCCCCCCC"));
  close2(create2("./debug/xCCCCCCCCC"));
  LGA_LOGGER_TEST("No diretorio raiz");
  printAllEntries(openDirectory);

  chdir2("debug");
  LGA_LOGGER_TEST("No diretorio debug");
  printAllEntries(openDirectory);

  printf("ptr[0] = %d\n",openDirectory.dataPtr[0] );
  printf("ptr[1] = %d\n",openDirectory.dataPtr[1] );
  LGA_LOGGER_TEST("Printando fill debug Bitmap de block:\n");
  printBitmap(BLOCK_TYPE, 100, 0 );
  LGA_LOGGER_TEST("Fim print fill debug Bitmap de block \n");

  printf("ptr[0] = %d\n",openDirectory.dataPtr[0] );
  printf("ptr[1] = %d\n",openDirectory.dataPtr[1] );
  printf("ptr[1] = %d\n",openDirectory.singleIndPtr );

  chdir2("/");
  delete2("/debug/A");
  delete2("/debug/AA");
  delete2("/debug/AAAA");
  delete2("/debug/AAAAA");
  delete2("/debug/AAAAAA");
  delete2("/debug/AAAAAAA");
  delete2("/debug/AAAAAAAAA");
  delete2("debug/B");
  delete2("debug/BB");
  delete2("debug/BBBB");
  delete2("debug/BBBBB");
  delete2("./debug/BBBBBB");
  delete2("./debug/BBBBBBB");
  delete2("./debug/BBBBBBBBB");
  delete2("./debug/C");
  delete2("./debug/CC");
  delete2("./debug/CCCC");
  delete2("./debug/CCCCC");
  delete2("./debug/CCCCCC");
  delete2("./debug/CCCCCCC");
  delete2("./debug/CCCCCCCCC");

  delete2("/debug/xAAAA");
  delete2("/debug/xAAAAA");
  delete2("/debug/xAAAAAA");
  delete2("/debug/xAAAAAAA");
  delete2("/debug/xAAAAAAAAA");
  delete2("debug/xB");
  delete2("debug/xBB");
  delete2("debug/xBBBB");
  delete2("debug/xBBBBB");
  delete2("./debug/xBBBBBB");
  delete2("./debug/xBBBBBBB");
  delete2("./debug/xBBBBBBBBB");
  delete2("./debug/xC");
  delete2("./debug/xCC");
  delete2("./debug/xCCCC");
  delete2("./debug/xCCCCC");
  delete2("./debug/xCCCCCC");
  delete2("./debug/xCCCCCCC");
  delete2("./debug/xCCCCCCCCC");

  rmdir2("/debug");
  LGA_LOGGER_TEST("No diretorio /");
  printAllEntries(openDirectory);

  LGA_LOGGER_TEST("Printando fill debug Bitmap de block:\n");
  printBitmap(BLOCK_TYPE, 100, 0 );
  LGA_LOGGER_TEST("Fim print fill debug Bitmap de block \n");
  return 0;
}
