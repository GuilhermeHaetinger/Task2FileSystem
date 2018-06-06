#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){
    int dirnameSize = 10;
    char dirname[dirnameSize];

    getcwd2(dirname, dirnameSize);

    printf("%s\n", dirname);

    mkdir2("directory");

    chdir2("directory");

    getcwd2(dirname, dirnameSize);

    printf("%s\n", dirname);

    return 0;
}
