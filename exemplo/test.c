#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"

int main(char* args[], int* argv){
    
    char super[SECTOR_SIZE];
    
    if(read_sector(0, super) == 0){
        LGA_LOGGER_TEST("Leu corretamente");
        printf("%c", ((struct t2fs_superbloco * )super)->id[0]);
        
    }else{
        LGA_LOGGER_TEST("Leu incorretamente");
    }
    
    return 0;
}
