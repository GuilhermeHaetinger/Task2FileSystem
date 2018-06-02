#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"

int main(){
    
    LGA_LOGGER_TEST("Creating file 'arquivoTeste'");
    
    FILE2 fileHandler = create2("arquivoTeste");

    if(fileHandler < 0){
        if(delete2("arquivoTeste") != 0){
            LGA_LOGGER_TEST("File exists and couldn't be deleted");
            return -1;
        }else{
            LGA_LOGGER_TEST("File exists and could be deleted");
            LGA_LOGGER_TEST("Creating File again");
            fileHandler = create2("arquivoTeste");
            if(fileHandler < 0){
               LGA_LOGGER_TEST("Create FAILED");
               return -1; 
            }
        }
    }
    LGA_LOGGER_TEST("Create SUCCEDED");

    
    return 0;
}
