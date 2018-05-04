#include "support.h"
#include "cdata.h"

#define EQUALS 0
#define SUCCEEDED 0
#define FAILED -1
#define END_CONTEXT 0
#define STACK_SIZE 163840

typedef struct s_BLOCK_RELEASER{
  int tid_block;
  int tid_releaser;
} BLOCK_RELEASER;

int LGA_tid_inside_of_fila(PFILA2 pFila, int tid);

int LGA_tid_remove_from_fila (PFILA2 pFila, int tid);

void* LGA_tid_get_from_fila (PFILA2 pFila, int tid);
