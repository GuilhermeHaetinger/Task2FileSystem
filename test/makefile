LIB_DIR=../lib
BIN_DIR=../bin
SRC_DIR=../src
TST_DIR=./
BUILD_DIR=build

create:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/create2test.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o create2test.c  -lm -Wall

support:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/supportTest.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o supportTest.c  -lm -Wall