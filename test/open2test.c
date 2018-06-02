#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"

int main(){
    char * filename = "filename";
    char * filename2 = "filename2";
    char * filename3 = "filename3";

    FILE2 handle1 = create2(filename);
    FILE2 handle2 = create2(filename2);
    close2(handle1);
    FILE2 handle3 = create2(filename3);

    return 0;
}
