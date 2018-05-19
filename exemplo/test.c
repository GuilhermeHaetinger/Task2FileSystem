#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"

int main(char* args[], int* argv){
    
    struct t2fs_superbloco * super = malloc(sizeof(struct t2fs_superbloco));

    if(read_sector(0, (char*)super)){
        LGA_LOGGER_TEST("Leu corretamente");
        printf("%c", super->id[0]);
        
    }else{
        LGA_LOGGER_ERROR("Leu incorretamente");
    }
    
    return 0;
}


