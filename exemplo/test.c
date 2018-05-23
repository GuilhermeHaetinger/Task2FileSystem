#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"

#define SECTOR_SIZE 256


int main(char* args[], int* argv){
    
    char * super = malloc(SECTOR_SIZE);

    if(read_sector(0, super) == 0){
        LGA_LOGGER_TEST("Leu corretamente");
        printf("%c", ((struct t2fs_superbloco * )super)->id[0]);
        
    }else{
        LGA_LOGGER_ERROR("Leu incorretamente");
    }
    
    return 0;
}
