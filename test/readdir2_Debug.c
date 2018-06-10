#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/bitmap2.h"
#include "../include/LGA_support.h"
#include "../include/t2fs.h"
#include "../include/LGA_logger.h"

int main () {

  initializeSuperBlock();
  printQuantBlock();
  printQuantInode();
  LGA_LOGGER_TEST("Creating directory");

  if (mkdir2("directory") < 0) {
    LGA_LOGGER_ERROR("mkdir2 Failed");
    return -1;
  }
  if (chdir2("directory") < 0) {
    LGA_LOGGER_ERROR("chdir2 Failed");
    return -1;
  }
  printQuantBlock();
  printQuantInode();
  LGA_LOGGER_TEST("Creating all files");

  char buffer[80];
  int handler;
  for (int i = 0; i < 1000; i ++) {
    snprintf(buffer, 80, "queijos%d.txt",i);
    handler = create2(buffer);
    if (handler < 0) {
      LGA_LOGGER_ERROR("Create2 Failed");
      break;
    } else {
      close2(handler);
    }
  }
  printQuantBlock();
  printQuantInode();

  if (chdir2("../") < 0) {
    LGA_LOGGER_ERROR("First .. chdir2 Failed");
    return -1;
  }

  DIR2 dirHandle = opendir2("directory");

  if (dirHandle < 0) {
    LGA_LOGGER_ERROR("opendir2 Failed");
    return -1;
  }

  DIRENT2 entry;

  LGA_LOGGER_TEST("Reading dir all files");

  for(int i =0; i < 1000; i++) {
    if (readdir2(dirHandle, &entry) < 0) {
      LGA_LOGGER_ERROR("readdir2 Failed");
      break;
    }
  }

  printQuantBlock();
  printQuantInode();

  if (rmdir2("directory") != SUCCEEDED) {
    LGA_LOGGER_ERROR("rmdir2 Failed");
  }
  if (chdir2("directory") < 0) {
    LGA_LOGGER_ERROR("Second directory chdir2 Failed");
    return -1;
  }

  LGA_LOGGER_TEST("Deleting all files");

  for (int i = 0; i < 1000; i ++) {
    snprintf(buffer, 80, "queijos%d.txt",i);
    handler = delete2(buffer);
    if (handler < 0) {
      LGA_LOGGER_ERROR("Delete2 Failed");
      break;
    } else {
      close2(handler);
    }
  }

  printQuantBlock();
  printQuantInode();

  LGA_LOGGER_TEST("Deleting dir");

  if (chdir2("../") < 0) {
    LGA_LOGGER_ERROR("chdir2 Failed");
    return -1;
  }
  if (rmdir2("directory") != SUCCEEDED) {
    LGA_LOGGER_ERROR("rmdir2 Failed");
    chdir2("directory");
    printAllEntries(openDirectory);
  }

  printQuantBlock();
  printQuantInode();

  return 0;
}
