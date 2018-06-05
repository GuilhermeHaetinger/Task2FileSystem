#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"
#include "../include/bitmap2.h"

/*-----------------------------------------------------------------------------
Fun��o: Usada para identificar os desenvolvedores do T2FS.
	Essa fun��o copia um string de identifica��o para o ponteiro indicado por "name".
	Essa c�pia n�o pode exceder o tamanho do buffer, informado pelo par�metro "size".
	O string deve ser formado apenas por caracteres ASCII (Valores entre 0x20 e 0x7A) e terminado por �\0�.
	O string deve conter o nome e n�mero do cart�o dos participantes do grupo.

Entra:	name -> buffer onde colocar o string de identifica��o.
	size -> tamanho do buffer "name" (n�mero m�ximo de bytes a serem copiados).

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int identify2 (char *name, int size){
    int i = 0;
	char identity[73] = {0};

	strcpy(identity, "Leonardo 00274721\nAndy Garramones 00274705\nGuilherme Haetinger 00274702\n");
	for(i=0;i<size && i < 73;i++) {
		name[i] = identity[i];
	}
  if(strcmp(identity,name) == EQUALS) {
    return SUCCEEDED;
  } else {
    LGA_LOGGER_ERROR("Size is smaller than the identification of the group");
    return FAILED;
  }
}


/*-----------------------------------------------------------------------------
Fun��o: Criar um novo arquivo.
	O nome desse novo arquivo � aquele informado pelo par�metro "filename".
	O contador de posi��o do arquivo (current pointer) deve ser colocado na posi��o zero.
	Caso j� exista um arquivo ou diret�rio com o mesmo nome, a fun��o dever� retornar um erro de cria��o.
	A fun��o deve retornar o identificador (handle) do arquivo.
	Esse handle ser� usado em chamadas posteriores do sistema de arquivo para fins de manipula��o do arquivo criado.

Entra:	filename -> nome do arquivo a ser criado.

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o handle do arquivo (n�mero positivo).
	Em caso de erro, deve ser retornado um valor negativo.
-----------------------------------------------------------------------------*/
FILE2 create2 (char *filename){

    LGA_LOGGER_DEBUG("[Entering create2]");
	if(initializeSuperBlock() != 0){
		LGA_LOGGER_ERROR("[create2] SuperBlock not properly initiated");
		return FAILED;
	}

	LGA_LOGGER_DEBUG("[create2] SuperBlock initialized");

	FileRecord file;
	LGA_LOGGER_DEBUG("[create2] Creating FileRecord");
	if(createRecord(filename, TYPEVAL_REGULAR, &file) != SUCCEEDED){
		return FAILED;
	}
	LGA_LOGGER_DEBUG("[create2] FileRecord created");

	LGA_LOGGER_LOG("[create2] Adding record to open file vector");
	FILE2 fileHandler = addFileToOpenFiles(file);
	if(fileHandler < SUCCEEDED){
		LGA_LOGGER_ERROR("[create2] File record isn't openable");
		return FAILED;
	}

	LGA_LOGGER_LOG("[create2] Creating inode");
	if(createRecordInode(file) != SUCCEEDED){
		return FAILED;
	}
	LGA_LOGGER_LOG("[create2] Inode created");

  	if (addFileToOpenDirectory(file) != SUCCEEDED) {
  	  LGA_LOGGER_ERROR("[create2] Failed to add file to directory");
  	  return FAILED;
  	}
  	LGA_LOGGER_LOG("[create2] Added file to directory");

  	return fileHandler;
}


/*-----------------------------------------------------------------------------
Fun��o:	Apagar um arquivo do disco.
	O nome do arquivo a ser apagado � aquele informado pelo par�metro "filename".

Entra:	filename -> nome do arquivo a ser apagado.

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int delete2 (char *filename){
    ///TODO
    LGA_LOGGER_DEBUG("[Entering delete2]");
    if(initializeSuperBlock() != 0){
  		LGA_LOGGER_ERROR("[delete2] SuperBlock not properly initiated");
  		return FAILED;
  	}
    LGA_LOGGER_DEBUG("[delete2] SuperBlock initialized");
    FileRecord record;
    int recordPosition, accessedPtr;

    //Procura pelo filename passado.
    //Recupera junto seu record, a sua position e em qual ponteiro do openDirectory foi encontrado
    if (getFileInode(filename, openDirectory, &record, &recordPosition, &accessedPtr) == NOT_FOUND) {
      return FAILED;
    }

    //Grava no disco em seu respectivo bloco o record com TYPEVAL_INVALIDO para setar como livre para ser usado
    record.TypeVal = TYPEVAL_INVALIDO;
    if(changeWriteBlock(openDirectory.dataPtr[accessedPtr], recordPosition*REGISTER_SIZE, (char*)&record, REGISTER_SIZE)!= SUCCEEDED){
  		LGA_LOGGER_ERROR("[delete2] Record not saved properly on its block");
  		return FAILED;
  	}

    //TODO aqui alguma especie de recursao para ir liberando também os inodes e possiveis indirecoes
    //TODO que o arquivo estivesse apontando
    //Para os inodes de fato
    // inodes->blocksFileSize = 0;
    // inodes->bytesFileSize = 0;
    // inodes->dataPtr[0] = INVALID_PTR;
    // inodes->dataPtr[1] = INVALID_PTR;

    //Seta no bitmap como livre o inode que o record estava apontando
    setBitmap2(INODE_TYPE, record.inodeNumber, INODE_FREE);

    return SUCCEEDED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Abre um arquivo existente no disco.
	O nome desse novo arquivo � aquele informado pelo par�metro "filename".
	Ao abrir um arquivo, o contador de posi��o do arquivo (current pointer) deve ser colocado na posi��o zero.
	A fun��o deve retornar o identificador (handle) do arquivo.
	Esse handle ser� usado em chamadas posteriores do sistema de arquivo para fins de manipula��o do arquivo criado.
	Todos os arquivos abertos por esta chamada s�o abertos em leitura e em escrita.
	O ponto em que a leitura, ou escrita, ser� realizada � fornecido pelo valor current_pointer (ver fun��o seek2).

Entra:	filename -> nome do arquivo a ser apagado.

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o handle do arquivo (n�mero positivo)
	Em caso de erro, deve ser retornado um valor negativo
-----------------------------------------------------------------------------*/
FILE2 open2 (char *filename){
	LGA_LOGGER_DEBUG("Entering open2");
	FileRecord file;
	Inode InodeBuffer;
	int position, accessedPtr;

	if (getFileInode(filename, openDirectory, &file, &position, &accessedPtr) == NOT_FOUND) {
		LGA_LOGGER_WARNING("File not found in this directory");
		return FAILED;
	}
	
	LGA_LOGGER_DEBUG("[open2] File found");

	LGA_LOGGER_LOG("[open2] Adding record to open file vector");
	FILE2 fileHandler = addFileToOpenFiles(file);
	if(fileHandler < SUCCEEDED){
		LGA_LOGGER_ERROR("[open2] File record isn't openable");
		return FAILED;
	}

	LGA_LOGGER_DEBUG("[open2] File opened properly");
    return SUCCEEDED;
}

/*-----------------------------------------------------------------------------
Fun��o:	Fecha o arquivo identificado pelo par�metro "handle".

Entra:	handle -> identificador do arquivo a ser fechado

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int close2 (FILE2 handle){
    LGA_LOGGER_DEBUG("Entering close2");
	if(removeFileFromOpenFiles(handle) != SUCCEEDED){
		LGA_LOGGER_ERROR("[close2] File couldn't be closed");
		return FAILED;
	}
	LGA_LOGGER_DEBUG("File closed");

    return SUCCEEDED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Realiza a leitura de "size" bytes do arquivo identificado por "handle".
	Os bytes lidos s�o colocados na �rea apontada por "buffer".
	Ap�s a leitura, o contador de posi��o (current pointer) deve ser ajustado para o byte seguinte ao �ltimo lido.

Entra:	handle -> identificador do arquivo a ser lido
	buffer -> buffer onde colocar os bytes lidos do arquivo
	size -> n�mero de bytes a serem lidos

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o n�mero de bytes lidos.
	Se o valor retornado for menor do que "size", ent�o o contador de posi��o atingiu o final do arquivo.
	Em caso de erro, ser� retornado um valor negativo.
-----------------------------------------------------------------------------*/
int read2 (FILE2 handle, char *buffer, int size){
    ///TODO
    return FAILED;
}

/*-----------------------------------------------------------------------------
Fun��o:	Realiza a escrita de "size" bytes no arquivo identificado por "handle".
	Os bytes a serem escritos est�o na �rea apontada por "buffer".
	Ap�s a escrita, o contador de posi��o (current pointer) deve ser ajustado para o byte seguinte ao �ltimo escrito.

Entra:	handle -> identificador do arquivo a ser escrito
	buffer -> buffer de onde pegar os bytes a serem escritos no arquivo
	size -> n�mero de bytes a serem escritos

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o n�mero de bytes efetivamente escritos.
	Em caso de erro, ser� retornado um valor negativo.
-----------------------------------------------------------------------------*/
int write2 (FILE2 handle, char *buffer, int size){
    ///TODO
    return FAILED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Fun��o usada para truncar um arquivo.
	Remove do arquivo todos os bytes a partir da posi��o atual do contador de posi��o (CP)
	Todos os bytes a partir da posi��o CP (inclusive) ser�o removidos do arquivo.
	Ap�s a opera��o, o arquivo dever� contar com CP bytes e o ponteiro estar� no final do arquivo

Entra:	handle -> identificador do arquivo a ser truncado

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int truncate2 (FILE2 handle){
    ///TODO
    return FAILED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Reposiciona o contador de posi��es (current pointer) do arquivo identificado por "handle".
	A nova posi��o � determinada pelo par�metro "offset".
	O par�metro "offset" corresponde ao deslocamento, em bytes, contados a partir do in�cio do arquivo.
	Se o valor de "offset" for "-1", o current_pointer dever� ser posicionado no byte seguinte ao final do arquivo,
		Isso � �til para permitir que novos dados sejam adicionados no final de um arquivo j� existente.

Entra:	handle -> identificador do arquivo a ser escrito
	offset -> deslocamento, em bytes, onde posicionar o "current pointer".

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int seek2 (FILE2 handle, DWORD offset){
    ///TODO
    return FAILED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Criar um novo diret�rio.
	O caminho desse novo diret�rio � aquele informado pelo par�metro "pathname".
		O caminho pode ser ser absoluto ou relativo.
	S�o considerados erros de cria��o quaisquer situa��es em que o diret�rio n�o possa ser criado.
		Isso inclui a exist�ncia de um arquivo ou diret�rio com o mesmo "pathname".

Entra:	pathname -> caminho do diret�rio a ser criado

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int mkdir2 (char *pathname){
	///
	///USE PARSER TO GET TO THE NEW OPEN DIRECTORY --- USING PATHNAME AS DIRECTORY NAME FOR NOW
	///
    LGA_LOGGER_DEBUG("[Entering mkdir2]");
	if(initializeSuperBlock() != 0){
		LGA_LOGGER_ERROR("[mkdir2] SuperBlock not properly initiated");
		return FAILED;
	}

	LGA_LOGGER_DEBUG("[mkdir2] SuperBlock initialized");

	FileRecord file;
	LGA_LOGGER_DEBUG("[mkdir2] Creating FileRecord");
	if(createRecord(pathname, TYPEVAL_DIRETORIO, &file) != SUCCEEDED){
		return FAILED;
	}
	LGA_LOGGER_DEBUG("[mkdir2] FileRecord created");

	LGA_LOGGER_LOG("[mkdir2] Creating inode");
	if(createDirectoryInode(file, openDirectoryFileRecord.inodeNumber) != SUCCEEDED){
		return FAILED;
	}
	LGA_LOGGER_LOG("[mkdir2] Inode created");

	if (addFileToOpenDirectory(file) != SUCCEEDED) {
	  LGA_LOGGER_ERROR("[mkdir2] Failed to add file to directory");
	  return FAILED;
	}
	LGA_LOGGER_LOG("[mkdir2] Added file to directory");

  return SUCCEEDED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Apagar um subdiret�rio do disco.
	O caminho do diret�rio a ser apagado � aquele informado pelo par�metro "pathname".
	S�o considerados erros quaisquer situa��es que impe�am a opera��o.
		Isso inclui:
			(a) o diret�rio a ser removido n�o est� vazio;
			(b) "pathname" n�o existente;
			(c) algum dos componentes do "pathname" n�o existe (caminho inv�lido);
			(d) o "pathname" indicado n�o � um arquivo;

Entra:	pathname -> caminho do diret�rio a ser criado

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int rmdir2 (char *pathname){
    ///TODO
    return FAILED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Altera o diret�rio atual de trabalho (working directory).
		O caminho desse diret�rio � informado no par�metro "pathname".
		S�o considerados erros:
			(a) qualquer situa��o que impe�a a realiza��o da opera��o
			(b) n�o exist�ncia do "pathname" informado.

Entra:	pathname -> caminho do novo diret�rio de trabalho.

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int chdir2 (char *pathname){
	///
	///USE PARSER TO GET TO THE NEW OPEN DIRECTORY --- USING PATHNAME AS DIRECTORY NAME FOR NOW
	///
  Inode currentDirectory = openDirectory;
  char **directoriesList; //Igual um argv
  int i, startingDir = 0, directories = parse(pathname, &directoriesList);

  //Caso o primeiro diretorio seja o / se seta para o openDirectory o rootDirectory
  if (directoriesList[0][0] == '/')
  {
    //TODO
    LGA_LOGGER_LOG("[chdir2] Comeca pelo raiz o  pathname");
    if(getRootInodeFile((char *)&openDirectory, (char *)&openDirectoryFileRecord) != SUCCEEDED){
      LGA_LOGGER_ERROR("[initializeSystem] Root inode not retrieved correctly");
      return FAILED;
    }
    //Nao precisa ler a primeira string parseada pois era / e já a atratamos
    startingDir = 1;
  }

  //Percorre toda o vetor de strings parseado do pathname e tentando entrar no diretorio atual
  LGA_LOGGER_LOG("[chdir2] Comeca pelo diretorio atual o pathname");
  for ( i = 0 + startingDir; i < directories; i++) {
    LGA_LOGGER_DEBUG("Entering chdir2");
    if(setNewOpenDirectory(directoriesList[i]) != SUCCEEDED){
      openDirectory = currentDirectory;
      freeList(&directoriesList,  directories);
      return FAILED;
    }
  }


  freeList(&directoriesList,  directories);
  return SUCCEEDED;


    return SUCCEEDED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Informa o diret�rio atual de trabalho.
		O "pathname" do diret�rio de trabalho deve ser copiado para o buffer indicado por "pathname".
			Essa c�pia n�o pode exceder o tamanho do buffer, informado pelo par�metro "size".
		S�o considerados erros:
			(a) quaisquer situa��es que impe�am a realiza��o da opera��o
			(b) espa�o insuficiente no buffer "pathname", cujo tamanho est� informado por "size".

Entra:	pathname -> buffer para onde copiar o pathname do diret�rio de trabalho
		size -> tamanho do buffer pathname

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getcwd2 (char *pathname, int size){
    ///TODO
    return FAILED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Abre um diret�rio existente no disco.
	O caminho desse diret�rio � aquele informado pelo par�metro "pathname".
	Se a opera��o foi realizada com sucesso, a fun��o:
		(a) deve retornar o identificador (handle) do diret�rio
		(b) deve posicionar o ponteiro de entradas (current entry) na primeira posi��o v�lida do diret�rio "pathname".
	O handle retornado ser� usado em chamadas posteriores do sistema de arquivo para fins de manipula��o do diret�rio.

Entra:	pathname -> caminho do diret�rio a ser aberto

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o identificador do diret�rio (handle).
	Em caso de erro, ser� retornado um valor negativo.
-----------------------------------------------------------------------------*/
DIR2 opendir2 (char *pathname){
    ///
	///USE PARSER TO GET TO THE NEW OPEN DIRECTORY --- USING PATHNAME AS DIRECTORY NAME FOR NOW
	///
	LGA_LOGGER_DEBUG("Entering opendir2");
	FileRecord record;
	Inode dir;
	int position, accessedPtr, inodePos;

	if (getFileInode(pathname, openDirectory, &record, &position, &accessedPtr) == NOT_FOUND) {
		LGA_LOGGER_WARNING("Directory not found in this directory");
		return FAILED;
	}
	
	LGA_LOGGER_DEBUG("[opendir2] Directory found");

	inodePos = record.inodeNumber;
	if(getInode(inodePos, (char*)&dir) != SUCCEEDED){
		LGA_LOGGER_ERROR("Couldn't retrieve directories Inode properly");
		return FAILED;
	}

	LGA_LOGGER_LOG("[opendir2] Adding record to open Directory vector");
	DIR2 dirHandler = addDirToOpenDirs(dir);
	if(dirHandler < SUCCEEDED){
		LGA_LOGGER_ERROR("[opendir2] Directory record isn't openable");
		return FAILED;
	}

	LGA_LOGGER_DEBUG("[opendir2] Directory opened properly");
    return SUCCEEDED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Realiza a leitura das entradas do diret�rio identificado por "handle".
	A cada chamada da fun��o � lida a entrada seguinte do diret�rio representado pelo identificador "handle".
	Algumas das informa��es dessas entradas devem ser colocadas no par�metro "dentry".
	Ap�s realizada a leitura de uma entrada, o ponteiro de entradas (current entry) deve ser ajustado para a pr�xima entrada v�lida, seguinte � �ltima lida.
	S�o considerados erros:
		(a) qualquer situa��o que impe�a a realiza��o da opera��o
		(b) t�rmino das entradas v�lidas do diret�rio identificado por "handle".

Entra:	handle -> identificador do diret�rio cujas entradas deseja-se ler.
	dentry -> estrutura de dados onde a fun��o coloca as informa��es da entrada lida.

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero ( e "dentry" n�o ser� v�lido)
-----------------------------------------------------------------------------*/
int readdir2 (DIR2 handle, DIRENT2 *dentry){
	LGA_LOGGER_DEBUG("entering readdir2");
    int num_of_entries = openDirectories[handle].dir.blocksFileSize * superBlock.blockSize / REGISTERS_PER_SECTOR;
	if(openDirectories[handle].entry == num_of_entries - 1){
		LGA_LOGGER_ERROR("[readdir2] Directory has run out of entries");
		return FAILED;
	}

	FileRecord entryDetails;
	Inode inodeDetails;

	if(getSpecificEntry(openDirectories[handle].dir, openDirectories[handle].entry,(char*)&entryDetails) != SUCCEEDED){
		LGA_LOGGER_ERROR("[readdir2] Failed to read entry");
		return FAILED;
	}
	
	if(getInode(entryDetails.inodeNumber, (char*)&inodeDetails) != SUCCEEDED){
		LGA_LOGGER_ERROR("[readdir2] Failed to read entry's Inode");
		return FAILED;
	}

	strcpy(dentry->name, entryDetails.name);
	dentry->fileType = entryDetails.TypeVal;
	dentry->fileSize = inodeDetails.bytesFileSize;

	openDirectories[handle].entry++;

	return SUCCEEDED;
}


/*-----------------------------------------------------------------------------
Fun��o:	Fecha o diret�rio identificado pelo par�metro "handle".

Entra:	handle -> identificador do diret�rio que se deseja fechar (encerrar a opera��o).

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int closedir2 (DIR2 handle){
    LGA_LOGGER_DEBUG("Entering closedir2");
	if(removeDirFromOpenDirs(handle) != SUCCEEDED){
		LGA_LOGGER_ERROR("[closedir2] Dir couldn't be closed");
		return FAILED;
	}
	LGA_LOGGER_DEBUG("[closedir2]File closed");

    return SUCCEEDED;
}
