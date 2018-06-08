LIB_DIR=../lib
BIN_DIR=../bin
SRC_DIR=../src
FRS_DAT=../fresh_dat
TST_DIR=./
BUILD_DIR=build

resetDatTest:
	cp $(FRS_DAT)/t2fs_disk.dat ./
fullDatTest:
	cp $(FRS_DAT)/t2fs_disk_sing_full.dat ./t2fs_disk.dat

create2Test:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/create2test.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o create2test.c  -lm -Wall

create_delete:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/create_delete.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o create_delete.c  -lm -Wall

openTest:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/open2test.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o open2test.c  -lm -Wall

supportTest:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/supportTest.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o supportTest.c  -lm -Wall

rootTest:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/rootTest.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o rootTest.c  -lm -Wall

directoryTest:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/directoryTest.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o directoryTest.c  -lm -Wall

getcw2:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/getcw2test.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o getcw2test.c  -lm -Wall

mkdir_chdirTest:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/mkdir_chdir.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o mkdir_chdir.c  -lm -Wall

chdir_sequence:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/chdir_sequence.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o chdir_sequence.c  -lm -Wall

chdir_recursive_relative:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/chdir_recursive_relative.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o chdir_recursive_relative.c  -lm -Wall

chdir_recursive_absolut:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/chdir_recursive_absolut.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o chdir_recursive_absolut.c  -lm -Wall

readdir:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/readdir2test.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o readdir2test.c  -lm -Wall

write:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/writeTest.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o writeTest.c  -lm -Wall

createTestSingDoub:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/createTestSingDoub.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o createTestSingDoub.c  -lm -Wall

createAndRemove:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/createAndRemove.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o createAndRemove.c  -lm -Wall

create_delete_path:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/create_delete_path.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o create_delete_path.c  -lm -Wall

mkdir_rmdir_path:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/mkdir_rmdir_path.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o mkdir_rmdir_path.c  -lm -Wall

open2_path:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/open2_path.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o open2_path.c  -lm -Wall

readdir_opendir_path:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/readdir_opendir_path.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o readdir_opendir_path.c  -lm -Wall

error_create_delete_path:
	rm -rf build
	mkdir $(BUILD_DIR)
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
	$(CC) -c $(SRC_DIR)/LGA_logger.c -o $(BIN_DIR)/LGA_logger.o
	$(CC) -c $(SRC_DIR)/LGA_support.c -o $(BIN_DIR)/LGA_support.o
	$(CC) -o $(BUILD_DIR)/error_create_delete_path.exe $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/LGA_logger.o $(BIN_DIR)/LGA_support.o $(BIN_DIR)/t2fs.o error_create_delete_path.c  -lm -Wall
