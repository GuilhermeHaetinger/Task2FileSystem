#include<stdbool.h>
#include "t2fs.h"
#include "apidisk.h"

#define EQUALS 0
#define SUCCEEDED 0
#define FAILED -1
#define ALREADY_INITIALIZED -2
#define END_CONTEXT 0
#define STACK_SIZE 163840

#define MAX_NUM_OF_OPEN_FILES 10
#define MAX_NUM_OF_OPEN_DIRECTORIES 50

#define INODE_SEARCH 0
#define BLOCK_SEARCH 1

#define INODE_SIZE 32


/*  tipos   */
typedef struct t2fs_superbloco SuperBlock;
typedef struct t2fs_record FileRecord;
typedef struct t2fs_inode Inode;

typedef struct RecordHandlerStruct{

    int CP;
    FileRecord file;

} recordHandler;

/*  dados   */
SuperBlock superBlock;
bool superBlockRead;


recordHandler openFiles[MAX_NUM_OF_OPEN_FILES];
recordHandler openDirectories[MAX_NUM_OF_OPEN_DIRECTORIES];

int inodeSectorIndex;


/*  funções   */

/*
 * Verifica se o superbloco já foi inicializado e, caso não, o inicializa
 * returns:
 * 0  -> caso tenha sido bem sucedido 
 * -1 -> caso tenha falhado 
*/
int initializeSuperBlock();


/*
 * Inicializa o superbloco
 * returns:
 * 0  -> caso tenha sido bem sucedido 
 * -1 -> caso tenha falhado 
 * -2 -> caso o superbloco já tenha sido inicializado
*/
int readSuperblock();


/*
 * Escreve um bloco de dados no disco
 * @params:
 * initialSector -> setor inicial de escrita
 * data          -> dados a serem escritos
 * returns:
 * 0  -> caso tenha sido bem sucedido 
 * -1 -> caso tenha falhado 
*/
int writeBlock(int initialSector, char* data);


/*
 * Lê um bloco de dados escrito no disco
 * @params:
 * initialSector -> setor inicial de leitura
 * data          -> espaço alocado para armazenamento dos dados lidos
 * returns:
 * 0  -> caso tenha sido bem sucedido 
 * -1 -> caso tenha falhado 
*/
int readBlock(int initialSector, char* data);


/*
 * Verifica se um bloco de dados pode se encaixar dentro dos blocos do sistema
 * @params:
 * data -> dados do bloco
 * returns:
 * true  -> caso seja
 * false -> caso não seja
*/
bool blockIsInAcceptableSize(char* data);


/*
 * Verifica se um bloco de dados pode se encaixar dentro dos blocos do sistema
 * @params:
 * indePos -> posição do inode
 * data -> dados do inode
 * returns:
 * 0  -> caso salve propriamente
 * -1 -> caso ocorra um erro
*/
int saveInode(DWORD inodePos, char* data);


/*
 * Verifica se um bloco de dados pode se encaixar dentro dos blocos do sistema
 * @params:
 * indePos -> posição do inode
 * data -> ponteiro para armazenamento de dados do inode
 * returns:
 * 0  -> caso recuere propriamente
 * -1 -> caso ocorra um erro
*/
int getSavedInode(DWORD inodePos, char* data);


/*
 * Tenta adicionar o arquivo ao vetor de arquivos abertos
 * @params:
 * file -> arquivo a ser adicionado
 * returns:
 *  >= 0 -> caso adicione propriamente (handler do registro)
 *  < 0  -> caso não há mais espaço
*/
FILE2 addFileToOpenFiles(FileRecord file);