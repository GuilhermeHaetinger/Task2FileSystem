#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"
#include "../include/apidisk.h"
#include "../include/t2fs.h"

/*
 * Cria o superbloco do sistema de arquivos no primeiro bloco.
 * @params:
 * blockSize -> quantos setores de 256 bytes que um bloco ocupa no sistema de arquivos
 * diskSize  -> quantos blocos existem no disco
 * returns:
 * 0  -> caso tenha sido bem sucedido 
 * -1 -> caso tenha falhado 
*/
int createSuperblock(WORD blockSize, DWORD diskSize){
  struct t2fs_superbloco *superbloco = malloc(sizeof(struct t2fs_superbloco));

  superbloco->id[0] = "T";
  superbloco->id[1] = "2";
  superbloco->id[2] = "F";
  superbloco->id[3] = "S";

  superbloco->version = 0x7E21;
  superbloco->superblockSize = 1;
  superbloco->freeBlocksBitmapSize = 1;
  superbloco->freeInodeBitmapSize = 1;
  superbloco->inodeAreaSize = 1;
  superbloco->blockSize = blockSize;
  superbloco->diskSize = diskSize;

  // Escreve em um setor, apenas, afinal o superbloco tem o tamanho < 256 bytes.
  // Apesar de ocupar um bloco, já sabemos quanto espaço ele vai tomar. Portanto: 
  if(write_sector(0, (char*)superbloco) == SUCCEEDED){
    return SUCCEEDED;
  }else{
    return FAILED;
  }

}

/*
 * Retorna o superbloco do sistema de arquivos
 * @params:
 * buffer -> ponteiro para a estrutura na qual o superbloco vai ser colocado
 * returns:
 * 0  -> caso tenha sido bem sucedido 
 * -1 -> caso tenha falhado 
*/
int getSuperblock(struct t2fs_superbloco * buffer){
  
  if(read_sector(0, (char*)buffer) == SUCCEEDED){
    return SUCCEEDED;
  }else{
    return FAILED;
  }
}

int writeBlock(int initialSector, char* data){
  int dataSize = sizeof(data);
  
  struct t2fs_superbloco *superBlock = malloc(sizeof(struct t2fs_superbloco));

  if(getSuperblock(superBlock) == SUCCEEDED){
    if(blockIsInAcceptableSize(data, superBlock)){
      int i;
      int dataSectors = ceil(dataSize/256);
      int writeResult = SUCCEEDED;

      for(i = 0; i < dataSectors; i++){
        if(writeResult == SUCCEEDED){
          writeResult = write_sector(initialSector + i, data + i*256);
        }else{
          return FAILED;
        }
      }
    }else{
      return FAILED;
    }
  }else{
    return FAILED;
  }

}

bool blockIsInAcceptableSize(char* data, struct t2fs_superbloco *superBlock){
  int dataSize = sizeof(data);

  int dataSectors = dataSize/256;

  if(dataSectors > superBlock->blockSize){
    return false;
  }else{
    return true;
  }
}