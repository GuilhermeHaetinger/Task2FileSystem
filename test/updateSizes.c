#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"

int main(){

    initializeSuperBlock();

    printf("root byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);
    mkdir2("directory1");
    updateAllSizes(openDirectory, openDirectoryFileRecord.inodeNumber, sizeof(DWORD));
    chdir2("directory1");
    printf("dir1 byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);
    mkdir2("directory2");
    updateAllSizes(openDirectory, openDirectoryFileRecord.inodeNumber, sizeof(DWORD));
    chdir2("directory2");
    printf("dir2 byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);
    mkdir2("directory3");
    updateAllSizes(openDirectory, openDirectoryFileRecord.inodeNumber, sizeof(DWORD));
    chdir2("directory3");
    printf("dir3 byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);
    mkdir2("directory4");
    updateAllSizes(openDirectory, openDirectoryFileRecord.inodeNumber, sizeof(DWORD));
    chdir2("directory4");
    printf("dir4 byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);
    mkdir2("directory5");
    updateAllSizes(openDirectory, openDirectoryFileRecord.inodeNumber, sizeof(DWORD));
    chdir2("directory5");
    printf("dir5 byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);

    FILE2 handle = create2("randomFile");

    int written = write2(handle, "I am writing this for test sakes.", 33);

    printf("current dir :: %s\n", openDirectoryFileRecord.name);

    updateAllSizes(openDirectory, openDirectoryFileRecord.inodeNumber, 100);//written+sizeof(DWORD));
    printf("adding %d to all directories\n", written+sizeof(DWORD));


    printf("dir5 byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);
    chdir2("../");
    printf("dir4 byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);
    chdir2("../");
    printf("dir3 byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);
    chdir2("../");
    printf("dir2 byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);
    chdir2("../");
    printf("dir1 byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);
    chdir2("../");
    printf("root byteSize :: %d\n blockSize :: %d\n", openDirectory.bytesFileSize, openDirectory.blocksFileSize);

    return 0;
}
