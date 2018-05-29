#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"

int main(){
    
    LGA_LOGGER_TEST("Creating file 'arquivoTeste'");
    
    FILE2 fileHandler = create2("arquivoTeste");
    if(fileHandler < 0){
        LGA_LOGGER_TEST("Create FAILED");
        return -1;
    }
    LGA_LOGGER_TEST("Create SUCCEDED");

    
    return 0;
}
