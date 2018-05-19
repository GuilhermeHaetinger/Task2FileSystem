#include "t2fs.h"
#include "apidisk.h"

#define EQUALS 0
#define SUCCEEDED 0
#define FAILED -1
#define END_CONTEXT 0
#define STACK_SIZE 163840

int createSuperBlock(WORD blockSize, DWORD diskSize);

int getSuperblock(struct t2fs_superbloco  *buffer);