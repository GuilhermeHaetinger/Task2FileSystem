#include<stdbool.h>
#include "t2fs.h"
#include "apidisk.h"

#define EQUALS 0
#define SUCCEEDED 0
#define FAILED -1
#define ALREADY_INITIALIZED -2
#define END_CONTEXT 0
#define STACK_SIZE 163840
#define NOT_FOUND  1
#define FOUND      0
#define MAX_NUM_OF_OPEN_FILES 10
#define MAX_NUM_OF_OPEN_DIRECTORIES 10

#define INODE_TYPE 0
#define BLOCK_TYPE 1

#define INODE_BUSY 1
#define INODE_FREE 0
#define INODE_SIZE 32

#define REGISTERS_PER_SECTOR 4

#define ROOT_INODE 0

#define REGISTER_SIZE 64

#define BLOCK_FULL -3
#define BLOCK_BUSY 1
#define BLOCK_FREE 0

/*  tipos   */
typedef struct t2fs_superbloco SuperBlock;
typedef struct t2fs_record FileRecord;
typedef struct t2fs_inode Inode;

typedef struct RecordHandlerStruct{

    int CP;
    FileRecord file;

} recordHandler;

typedef struct DirectoryHandlerStruct{

    int entry;
    Inode dir;

} directoryHandler;

/*  dados   */
SuperBlock superBlock;
bool superBlockRead;
FileRecord closedRecord;
Inode closedDir;


recordHandler openFiles[MAX_NUM_OF_OPEN_FILES];
directoryHandler openDirectories[MAX_NUM_OF_OPEN_DIRECTORIES];
Inode openDirectory;
char* openDirName;
FileRecord openDirectoryFileRecord;


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
 * Retorna a posição do bloco de dados dentro do disco
 * @params:
 * dataBlockPos  -> posição do bloco dentro do bitmap
 * returns:
 * >0  -> posição do bloco
 * -1 -> caso tenha falhado
*/
int getDataBlockPosition(int dataBlockPos);


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
 * Tenta adicionar o diret´orio ao vetor de diretórios abertos
 * @params:
 * dir -> diretório a ser adicionado
 * returns:
 *  >= 0 -> caso adicione propriamente (handler do diretório)
 *  < 0  -> caso não há mais espaço
*/
DIR2 addDirToOpenDirs(Inode dir);

/*
 * Remove o arquivo aberto do vetor de arquivos abertos
 * @params:
 * handler -> índice do arquivo no vetor
 * returns:
 *  >= 0 -> caso remova propriamente
 *  < 0  -> caso não consiga
*/
int removeFileFromOpenFiles(FILE2 handler);

/*
 * Remove o diretório aberto do vetor de diretórios abertos
 * @params:
 * handler -> índice do diretório no vetor
 * returns:
 *  >= 0 -> caso remova propriamente
 *  < 0  -> caso não consiga
*/
int removeDirFromOpenDirs(DIR2 handler);

/*
 * Abstração da criação de Records - tanto para diretórios quanto para regular files
 * @params:
 * name -> nome do novo arquivo
 * typeVal -> valor do tipo do arquivo
 * fileRecord -> buffer de armazenamento dos dados do Record
 * returns:
 *  >= 0 -> caso crie propriamente
 *  < 0  -> caso não
*/
int createRecord(char * name, BYTE typeVal, FileRecord * fileRecord);

/*
 * Criação de inode para um RecordFile
 * @params:
 * file -> Record para o qual criaremos um inode
 * returns:
 *  >= 0 -> caso crie propriamente
 *  < 0  -> caso não
*/
int createRecordInode(FileRecord file);

/*
 * Criação de inode para um Diretório
 * @params:
 * file -> Record para o qual criaremos um inode
 * fatherInodeNumber -> Numero do inode do pai desse diretório
 * returns:
 *  >= 0 -> caso crie propriamente
 *  < 0  -> caso não
*/
int createDirectoryInode(FileRecord file, int fatherInodeNumber);

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
void changeSectorInode(int start, char* data, int dataSize, char* diskSector, char* saveSector);

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
 * inodeBuffer -> Buffer para receber o iNode
 * fileBuffer -> Buffer para receber o Record File
 * returns:
 * 0  -> caso tenha sido bem sucedido
 * -1 -> caso tenha falhado
*/
int getRootInodeFile(char* inodeBuffer, char* fileBuffer);

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
 * Pega um registerFile de um buffer dado
 * * @params:
 * registerNumber -> Posicao do registro dentro do setor
 * diskBuffer -> Buffer que contem as informacoes do disco
 * diskBufferSize -> Tamanho do disBuffer
 * buffer -> Vetor que recebera o register
 * returns:
 * 0  -> caso tenha sido bem sucedido
 * -1 -> caso tenha falhado
*/
int getRegisterFile(int registerNumber, char* diskBuffer, int diskBufferSize, char *buffer);

/*
 * Pega os dados de um diskSector e coloca no saveSector
 * * @params:
 * buffer -> Buffer para receber os dados
 * start -> Ponto inicial do daoo no setor
 * dataSize -> Tamanho do dado a ser pego
 * diskBuffer -> Setor a ter os dados pegos
 * diskSize   -> Tamanho do disco
 * returns:
 * 0  -> caso tenha sido bem sucedido
 * -1 -> caso tenha falhado
*/
int getDataFromDisk(char *buffer, int start, int dataSize, char* diskBuffer, int diskSize);

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

/*
 * aloca os blocos para posicionar os registros
 * *@params:
 * ptr -> Recebe o ponteiro de um inteiro que tem o valor de um ponteiro
 * newBlock -> Ponteiro para um inteiro usado como boolean se for alocado novo bloco
 * returns:
 * >0  -> posicao
 * -1  -> caso tenha falhado
 * -2  -> caso o o ponteiro do bloco ja esteja cheio
*/
int searchNewFileRecordPosition(DWORD *ptr, int *newBlock);

/*
 * escreve no bloco a data sem esvaziar o bloco inteiro
 * *@params:
 * blockPos -> Posicao do bloco
 * dataPos -> A partir de onde a data sera escrita
 * data -> Data a ser escrita
 * dataSize -> Tamanho da data
 * returns:
 * 0  ->  sucesso
 * -1  -> caso tenha falhado
*/
int changeWriteBlock(int blockPos, int dataPos, char* data, int dataSize);

/*
 * Copia o que tem no disco, substituindo os dados nas posicoes corretas
 * sem apagar o disco
 * *@params:
 * start -> a partir de qual posicao vai escrever
 * data -> data a ser escrita
 * dataSize -> tamanho da data
 * diskSector -> buffer do dano a ser modificado
 * diskSize -> tamanho do buffer
 * saveSector -> buffer que recebera a modificacao
 * returns:
 * void
*/
void changeDisk(int start, char* data, int dataSize, char* diskSector, int diskSize, char* saveSector);

/*
 * Escreve um FileRecord em um inode
 * *@params:
 * inode -> inode
 * buffer -> FileRecord a ser escrito
 * position -> Posicao do FileRecord
 * returns:
 * 0  ->  sucesso
 * -1  -> caso tenha falhado
*/
int writeFilePositionInInode(Inode inode, char *buffer, int position);

/*
 * Printa todas as entradas de um iNode
 * *@params:
 * inode -> inode
 * returns:
 * 0  ->  sucesso
 * -1  -> caso tenha falhado
*/
int printAllEntries(Inode inode);

/*
 * Printa as entradas de um ponteiro
 * *@params:
 * ptr -> Ponteiro para um bloco de registros
 * returns:
 * 0  ->  sucesso
 * -1  -> caso tenha falhado
*/
int _printEntries(DWORD ptr);

/*
 * Recupera arquivo de entry específica dentro do diretório
 * *@params:
 * entryNum -> índice do arquivo dentro do inode
 * buffer   -> buffer de armazenamento do record
 * returns:
 * 0  ->  sucesso
 * -1  -> caso tenha falhado
*/
int getSpecificEntry(Inode dir, int entryNum, char* buffer);

DWORD getDirFilenameInode(char* filename, Inode inode);

DWORD _getDirFilenameInode(DWORD ptr, char* filename);

int _getFileInode(DWORD ptr, char* filename, FileRecord * fileInode, int *position);

int getFileInode(char* filename, Inode inode, FileRecord * fileInode, int *position, int *accessedPtr);

/*
 * Entra em um diretório específico dentro do diretório atual
 * @params:
 * directoryName -> nome do diretório
 * returns:
 * 0  ->  sucesso
 * -1  -> caso tenha falhado
*/
int setNewOpenDirectory(char * directoryName);

int findFileRecordOnDirectory(char * file);

FILE2 findProperPositionOnOpenFiles();

DIR2 findProperPositionOnOpenDirectories();

int initializeOpenFiles();

int initializeOpenFilesAndDirectories();

/*
 * Receive one path string and fills a given ***char with each directory
 * @params {String, List}
 * String --> being string a char[]
 * List -->  being list a char*** (a pointer to an argv)
 * Returns:
 * Number of strings filled in the list
 * Access list as an array of strings ("%s", list[1])
*/
int parse (char * string, char ***lista);

/*
 * Recebe uma lista de strings e a quantidade de strings que ela possui e da free
 * @params {List, words}
 * String --> being string a char[]
 * words -->  being words an integer
*/
void freeList(char ***lista, int words);

int _searchNewFileRecordPosition(DWORD ptr,int *newBlock);

/*
 * Recebe uma lista de strings e a quantidade de strings que ela possui e da free
 * @params {List, words}
 * CP --> Contador de posicao
 * fileInode -> arquivo em questao
*/
int invalidateFromCPOn(DWORD CP, Inode fileInode);

int writeOnFile(Inode *fileInode, int CP, char * content, int contentSize);

int getByteBlockPositioning(Inode * fileInode, int byte, int * block, int * forwardOffset, int * backwardsOffset, int contentSize, int contentWritten);

int readFileBlocks(Inode fileInode, int CP, char * buffer, int contentSize);

/*
 * Recebe um ponteiro indireto (com endereco) e retorna uma posicao entre [SECOND_REG + 1, SINGLE_PTR]
 * @params
 * singleIndPtr --> Ponteiro indireto
 * newBlock --> Debugador do Andy
 * returns:
 * >= 0  -> sucesso, posição
 * -3   -> Bloco full
 * -1  -> caso tenha falhado
*/

int singleIndGetPos(DWORD *singleIndPtr, int *newBlock);

/*
 * Escreve o registro no bloco correto
 * @params
 * singleIndPtr --> Ponteiro indireto
 * position --> Posição do registro
 * fileRecord --> registro a ser gravado
 * returns:
 *  0  -> sucesso
 * -1  -> caso tenha falhado
*/
int singleIndWrite(DWORD singleIndPtr, int position, char * fileRecord);

/*
 * Printa todas as entradas
 * @params
 * singleIndPtr --> Ponteiro indireto
 * returns:
 *  0  -> sucesso
 * -1  -> caso tenha falhado
*/
int singleIndPrint(DWORD singleIndPtr);

/*
 * Printa todas as entradas
 * @params
 * doubleIndPtr --> Ponteiro duplo indireto
 * returns:
 *  0  -> sucesso
 * -1  -> caso tenha falhado
*/
int doubleIndPrint(DWORD doubleIndPtr);

/*
 * Recebe um ponteiro indireto duplo (com endereco) e retorna uma posicao entre [SINGLE_PTR + 1, DOUBLE_PTR]
 * @params
 * doubleIndPtr --> Ponteiro duplo indireto
 * newBlock --> Debugador do Andy
 * returns:
 * >= 0  -> sucesso, posição
 * -3   -> Bloco full
 * -1  -> caso tenha falhado
*/
int doubleIndGetPos( DWORD *doubleIndPtr, int *newBlock);

/*
 * Escreve o registro no bloco correto
 * @params
 * doubleIndPtr --> Ponteiro duplo indireto
 * position --> Posição do registro
 * fileRecord --> registro a ser gravado
 * returns:
 *  0  -> sucesso
 * -1  -> caso tenha falhado
*/
int doubleIndWrite(DWORD doubleIndPtr, int position, char * fileRecord);

int _searchNewFileRecordPosition(DWORD ptr,int *newBlock);

/*
 * Printa os bytes de um bloco
 * @params
 * blockPos --> Posição do bloco
*/
void printBlock(DWORD blockPos);

/*
 * Limpa um bloco com 0's
 * @params
 * blockPos --> Posição do bloco
 * returns:
 *  0  -> sucesso
 * -1  -> caso tenha falhado
*/
int cleanBlock(DWORD blockPos);

int _getFileInodeSingInd(DWORD singleIndPtr, char* filename, FileRecord * fileInode,int *position);

int _getFileInodeDoubleInd(DWORD doubleIndPtr, char* filename, FileRecord * fileInode,int *position);

/*
 * Remove todos os blocos apontados pelo inode
 * @params
 * inodePos --> Posição do inode
 * returns:
 *  0  -> sucesso
 * -1  -> caso tenha falhado
*/
int removeInode(DWORD inodePos);

/*
 * Remove todos os blocos indiretamente apontados por ele
 * @params
 * singleIndPtr --> Ponteiro indireto
 * returns:
 *  0  -> sucesso
 * -1  -> caso tenha falhado
*/
int _removeInode_SingleInd(DWORD singleIndPtr);

/*
  * Remove todos os blocos indiretamente apontados por ele
 * @params
 * doubleIndPtr --> Ponteiro duplo indireto
 * returns:
 *  0  -> sucesso
 * -1  -> caso tenha falhado
*/
int _removeInode_DoubleInd(DWORD doubleIndPtr);
