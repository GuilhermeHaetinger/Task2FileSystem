#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

  initializeSuperBlock();
  printAllEntries(openDirectory);
  chdir2("dir1");
  printAllEntries(openDirectory);
  chdir2("dir11");
  printAllEntries(openDirectory);
  chdir2("/dir1/dir12");
  printAllEntries(openDirectory);
  chdir2("/dir2");
  printAllEntries(openDirectory);
  chdir2("/dir2/dir21");
  printAllEntries(openDirectory);

  chdir2("/");
  //Sao usados 11 inodes padrao do .dat
  printf("\n" );
  printBitmap(INODE_TYPE, 15, 1 );

  printf("\nRemovendo 2 arquivos\n\n" );
  delete2("/dir1/dir11/file111");
  delete2("./file3");
  printBitmap(INODE_TYPE, 15, 1 );
  printf("\nCriando 2 arquivos\n\n" );
  create2("/Etois.txt");
  create2("./Vamoo.sdp");
  printBitmap(INODE_TYPE, 15, 1 );
  return 0;
}
