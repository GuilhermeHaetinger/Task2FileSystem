#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/bitmap2.h"
#include "../include/LGA_support.h"
#include "../include/LGA_logger.h"

int inodeWritingTest();
int blockWritingTest();

int main(){
  create2("oioioioioi.txt");
  delete2("oioioioioi.txt");
  // WORD inodePos Andy.txt = 13
  // Inode fileInode Andy.txt = 0xffdd3ebc
  // setor inode Andy.txt = 13
  // offset setor inodePos Andy.txt = 13
  printAllEntries(openDirectory);
}
