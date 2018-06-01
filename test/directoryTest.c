#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/bitmap2.h"
#include "../include/LGA_support.h"
#include "../include/LGA_logger.h"

int inodeWritingTest();
int blockWritingTest();

int main(){
  create2("Meu_Arquivo");
  printAllEntries(openDirectory);
}
