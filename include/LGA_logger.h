#define LGA_DEBUG 4
#define LGA_LOG 3
#define LGA_WARNING 2
#define LGA_ERROR 1
#define LGA_ENABLE 1
#define LGA_UNABLE 0

/* COLORS */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define LGA_LOGGER_STATUS_ENV LGA_GET_ENV_LOGGER()
#define LGA_LOGGER_TEST_ENV LGA_GET_ENV_LOGGER_TEST()
#define LGA_LOGGER_IMPORTANT_ENV LGA_GET_ENV_LOGGER_IMPORTANT()

void LGA_LOGGER_LOG(char * text);

void LGA_LOGGER_TEST(char * text);

void LGA_LOGGER_ERROR(char * text);

void LGA_LOGGER_WARNING(char * text);

void LGA_LOGGER_IMPORTANT(char * text);

void LGA_LOGGER_DEBUG(char * text);

int LGA_GET_ENV_LOGGER();

int LGA_GET_ENV_LOGGER_TEST();

int LGA_GET_ENV_LOGGER_IMPORTANT();
