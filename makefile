#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
#

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src

all: t2fs libt2fs

t2fs:
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o

libt2fs:
	ar crs $(LIB_DIR)/libt2fs.a $(BIN_DIR)/t2fs.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~
