#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/bitmap2.h"
#include "../include/LGA_support.h"
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"

int main(){

  printAllEntries(openDirectory);

  if (mkdir2("directoryOnRoot") < 0) {
    LGA_LOGGER_TEST("mkdir2 Failed");
    return -1;
  }

  if (chdir2("directoryOnRoot") < 0) {
    LGA_LOGGER_TEST("chdir2 Failed");
    return -1;
  }

  if (mkdir2("/directoryOnRoot/directory") < 0) {
    LGA_LOGGER_TEST("mkdir2 Failed");
    return -1;
  }
  if (chdir2("/directoryOnRoot") < 0) {
    LGA_LOGGER_TEST("chdir2 Failed");
    return -1;
  }

  if (create2("./directory/create2teste.txt") < 0) {
    LGA_LOGGER_TEST("Create2 Failed");
    return -1;
  }
  if (create2("/directoryOnRoot/directory/sample1") < 0) {
    LGA_LOGGER_TEST("Create2 Failed");
    return -1;
  }
  if (create2("directory/sample2") < 0) {
    LGA_LOGGER_TEST("Create2 Failed");
    return -1;
  }
  if (create2("./directory/sample3") < 0) {
    LGA_LOGGER_TEST("Create2 Failed");
    return -1;
  }
  if (chdir2("../") < 0) {
    LGA_LOGGER_TEST("chdir2 Failed");
    return -1;
  }

  DIR2 dirHandle = opendir2("/directoryOnRoot/directory");
  if (dirHandle < 0) {
    LGA_LOGGER_TEST("opendir2 Failed");
    return -1;
  }

  DIRENT2 entry;

  if (readdir2(dirHandle, &entry) < 0) {
    LGA_LOGGER_TEST("readdir2 Failed");
    return -1;
  }
  printf("%s \n", entry.name);
  if (readdir2(dirHandle, &entry) < 0) {
    LGA_LOGGER_TEST("readdir2 Failed");
    return -1;
  }
  printf("%s \n", entry.name);
  if (readdir2(dirHandle, &entry) < 0) {
    LGA_LOGGER_TEST("readdir2 Failed");
    return -1;
  }
  printf("%s \n", entry.name);
  if (readdir2(dirHandle, &entry) < 0) {
    LGA_LOGGER_TEST("readdir2 Failed");
    return -1;
  }
  printf("%s \n", entry.name);
  if (readdir2(dirHandle, &entry) < 0) {
    LGA_LOGGER_TEST("readdir2 Failed");
    return -1;
  }
  printf("%s \n", entry.name);
  if (readdir2(dirHandle, &entry) < 0) {
    LGA_LOGGER_TEST("readdir2 Failed");
    return -1;
  }
  printf("%s \n", entry.name);

  if(strcmp(entry.name, "sample3") != 0){
    LGA_LOGGER_TEST("readdir didn't return the expected value");
    return -1;
  }

  if(closedir2(dirHandle) != 0){
    LGA_LOGGER_TEST("dir failed while closing");
    return -1;
  }

  if(openDirectories[dirHandle].entry != -1){
    LGA_LOGGER_TEST("dir not closed properly");
    return -1;
  }

  LGA_LOGGER_TEST("TEST SUCCEEDED");
  return 0;
}
