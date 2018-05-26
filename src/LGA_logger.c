#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/LGA_logger.h"


/**
 * Log a normal Log to the final user
 * @param text LogText
 */
void LGA_LOGGER_LOG(char * text) {

  if(LGA_LOGGER_STATUS_ENV >= LGA_LOG) {
    char *debugText = calloc(strlen(text) + 6, sizeof(char));
    strcat(debugText, text);
    strcat(debugText, "\n");

    printf(ANSI_COLOR_BLUE "[LOG] " ANSI_COLOR_RESET);
    printf("%s", debugText);

    free(debugText);
  }
};

/**
 * Log a Test to the final user
 * @param text TestText
 */
void LGA_LOGGER_TEST(char * text) {
  if(LGA_LOGGER_TEST_ENV == LGA_ENABLE) {
    char *debugText = calloc(strlen(text) + 6, sizeof(char));
    strcat(debugText, text);
    strcat(debugText, "\n");

    printf(ANSI_COLOR_GREEN "[TEST] " ANSI_COLOR_RESET);
    printf("%s", debugText);

    free(debugText);
  }
};

/**
 * Log a Error to the final user
 * @param text ErrorText
 */
void LGA_LOGGER_ERROR(char * text) {
  if(LGA_LOGGER_STATUS_ENV >= LGA_ERROR) {
    char *debugText = calloc(strlen(text) + 6, sizeof(char));
    strcat(debugText, text);
    strcat(debugText, "\n");

    printf(ANSI_COLOR_RED "[ERROR] ");
    printf("%s" ANSI_COLOR_RESET, debugText);

    free(debugText);
  }
};

/**
 * Log a Warning to the final user
 * @param text WarningText
 */
void LGA_LOGGER_WARNING(char * text) {
  if(LGA_LOGGER_STATUS_ENV >= LGA_WARNING) {
    char *debugText = calloc(strlen(text) + 6, sizeof(char));
    strcat(debugText, text);
    strcat(debugText, "\n");

    printf(ANSI_COLOR_MAGENTA "[WARNING] " ANSI_COLOR_RESET);
    printf("%s", debugText);

    free(debugText);
  }
};

/**
 * Log an Important Log to the final user
 * @param text WarningText
 */
void LGA_LOGGER_IMPORTANT(char * text) {
  if(LGA_LOGGER_IMPORTANT_ENV == LGA_ENABLE) {
    char *debugText = calloc(strlen(text) + 6, sizeof(char));
    strcat(debugText, text);
    strcat(debugText, "\n");

    printf(ANSI_COLOR_YELLOW "[IMPORTANT] " ANSI_COLOR_RESET);
    printf("%s", debugText);

    free(debugText);
  }
};

/**
 * Log a Debug Log to the final user
 * @param text WarningText
 */
void LGA_LOGGER_DEBUG(char * text) {
  if(LGA_LOGGER_STATUS_ENV >= LGA_DEBUG) {
    char *debugText = calloc(strlen(text) + 6, sizeof(char));
    strcat(debugText, text);
    strcat(debugText, "\n");

    printf(ANSI_COLOR_CYAN "[DEBUG] " ANSI_COLOR_RESET);
    printf("%s", debugText);

    free(debugText);
  }
};

/**
 * Get the LGA_LOGGER Environment variable
 * @return [description]
 */
int LGA_GET_ENV_LOGGER() {
  if(getenv("LGA_LOGGER") != NULL) {
    return (int)atol(getenv("LGA_LOGGER"));
  }
  return 0;
}

/**
 * Get the LGA_LOGGER_TEST Environment variable
 * @return [description]
 */
int LGA_GET_ENV_LOGGER_TEST() {
  if(getenv("LGA_LOGGER_TEST") != NULL) {
    return (int)atol(getenv("LGA_LOGGER_TEST"));
  }
  return 0;
}

/**
 * Get the LGA_LOGGER_IMPORTANT Environment variable
 * @return [description]
 */
int LGA_GET_ENV_LOGGER_IMPORTANT() {
  if(getenv("LGA_LOGGER_IMPORTANT") != NULL) {
    return (int)atol(getenv("LGA_LOGGER_IMPORTANT"));
  }
  return 0;
}
