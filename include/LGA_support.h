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

#define INODE_TYPE 0
#define BLOCK_TYPE 1

#define INODE_BUSY 1
#define INODE_FREE 0
#define INODE_SIZE 32

#define ROOT_INODE 0

#define REGISTER_SIZE 64

#define ENTRY_ENABLED -2

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
Inode openDirectory;


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
 * Procura um bloco disponível no disco
 * returns:
 * >0  -> posição do bloco
 * -1  -> caso tenha falhado
*/
int getFreeBlock();

/*
 * Escreve um bloco de dados no disco
 * @params:
 * blockPos -> posição do bloco
 * data      -> espaço alocado para armazenamento dos dados lidos
 * dataSize  -> tamanho do data
 * returns:
 * 0  -> caso tenha sido bem sucedido
 * -1 -> caso tenha falhado
*/
int writeBlock(int blockPos, char* data, int dataSize);


/*
 * Lê um bloco de dados escrito no disco
 * @params:
 * blockPos  -> posição do block
 * data      -> espaço alocado para armazenamento dos dados lidos
 * dataSize  -> tamanho do data
 * returns:
 * 0  -> caso tenha sido bem sucedido
 * -1 -> caso tenha falhado
*/
int readBlock(int blockPos, char* data, int dataSize);


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
 * Verifica se um bloco de inodes pode se encaixar dentro dos blocos do sistema
 * @params:
 * indePos -> posição do inode
 * data -> dados do inode
 * returns:
 * 0  -> caso salve propriamente
 * -1 -> caso ocorra um erro
*/
int saveInode(DWORD inodePos, char* data);

/*
 * Encaixa um bloco de inodes dentro dos blocos do sistema
 * @params:
 * indePos -> posição do inode
 * data -> dados do inode
 * returns:
 * 0  -> caso salve propriamente
 * -1 -> caso ocorra um erro
*/
int setInode(DWORD inodePos, char* data);

/*
 * Verifica se um bloco de dados pode se encaixar dentro dos blocos do sistema
 * @params:
 * indePos -> posição do inode
 * data -> ponteiro para armazenamento de dados do inode
 * returns:
 * 0  -> caso recuere propriamente
 * -1 -> caso ocorra um erro
*/
int getInode(DWORD inodePos, char* data);

/*
 * Tenta adicionar o arquivo ao vetor de arquivos abertos
 * @params:
 * file -> arquivo a ser adicionado
 * returns:
 *  >= 0 -> caso adicione propriamente (handler do registro)
 *  < 0  -> caso não há mais espaço
*/
FILE2 addFileToOpenFiles(FileRecord file);

/*
 * Busca a posição do primeiro inode free
 * returns:
 * int -> posição no bitmap
*/
int getFreeInode();

/*
 * creates inode
 * returns:
 * void
*/
void initializeInode(Inode * buffer);

/*
 * Busca o numero do sector de um iNode
 * * @params:
 * inodePos -> Posição do iNode na ordem dos iNodes
 * returns:
 * int -> posição do setor no disco
*/
int getSectorIndexInode(DWORD inodePos);

/*
 * Busca o numero do offset de um iNode
 * * @params:
 * inodePos -> Posição do iNode na ordem dos iNodes
 * returns:
 * int -> offset
*/
int getOffsetInode(DWORD inodePos);

/*
 * Muda um setor deixando os dados originais que nao serao substituidos como estavam
 * * @params:
 * inodePos -> Posição do iNode na ordem dos iNodes
 * start -> indice inicial do dado que sera inserido
 * data -> dado a ser inserido
 * diskSector -> setor original
 * saveSector -> setor a ser salvo
*/
void changeSector(int start, char* data, int dataSize, char* diskSector, char* saveSector);

/*
 * Limpa um array com 0 no lugar dos valores.
 * * @params:
 * array -> Array a ser limpado
 * size -> Tamanho do array
*/
void cleanArray(char *array, int size);

/*
 * Cria o iNode do root
 * returns:
 * 0  -> caso tenha sido bem sucedido
 * -1 -> caso tenha falhado
*/
int createRoot();

/*
 * Verifica se o inode do Root esta criado
 * returns:
 * 0  -> caso ja esteja criado
 * -1 -> caso nao tenha sido criado
*/
int rootCreated();

/*
 * Pega o inode Root
 * * @params:
 * buffer -> Buffer para receber o iNode
 * returns:
 * 0  -> caso tenha sido bem sucedido
 * -1 -> caso tenha falhado
*/
int getRootInode(char* buffer);

/*
 * Concatena os dados de dois char*
 * * @params:
 * concatened -> Vetor final que tera a concatenacao
 * concatStartPos -> Posicao inicial para comecar a concatencao
 * buffer -> Vetor que sera concatenado
 * bufferSize -> Tamanho do buffer
*/
void concatCustom(char* concatened, int concatStartPos, char* buffer, int bufferSize);

/*
 * Pega um registerFile de um setor
 * * @params:
 * sectorPos -> Posicao do setor
 * registerNumber -> Posicao do registro dentro do setor
 * buffer -> Vetor que recebera o register
 * returns:
 * 0  -> caso tenha sido bem sucedido
 * -1 -> caso tenha falhado
*/
int getRegisterFile(int sectorPos, int registerNumber, char *buffer);

/*
 * Pega os dados de um diskSector e coloca no saveSector
 * * @params:
 * start -> Ponto inicial do daoo no setor
 * dataSize -> Tamanho do dado a ser pego
 * diskSector -> Setor a ter os dados pegos
 * saveSector -> Buffer para receber os dados
 * returns:
 * 0  -> caso tenha sido bem sucedido
 * -1 -> caso tenha falhado
*/
int getDataSector(int start, int dataSize, char* diskSector, char* buffer);

/*
 * Salva o arquivo no diretório atual
 * * @params:
 * file -> FileRegister a ser adicionado
 * returns:
 * 0  -> caso tenha sido bem sucedido
 * -1 -> caso tenha falhado
*/
int addFileToOpenDirectory(FileRecord file);

/*
 * Verifica o melhor local para adicionar um FileRegister no diretório atual
 * returns:
 * >0  -> posição
 * -1  -> caso tenha falhado
*/
int getNewFilePositionOnOpenDirectory();

/*
 * aloca os blocos para posicionar os registros 
 * *@params:
 * inode -> inode em questão
 * returns:
 * 0  -> bem sucedido
 * -1  -> caso tenha falhado
*/
int allocateDataBlock(Inode inode);