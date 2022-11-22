#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define MAXLEN 256

#define DEFAULT_MODE 20
#define BACKGROUND_MODE 21
#define ZERO_MODE 22
#define ONE_MODE 23

#define NOOPERATOR 30

#define WORD 0
#define CONVEYOR 1
#define AND 2
#define OR 3
#define AMPERSAND 4
#define SEMICOLON 5
#define RIGHT_ARROW 6
#define RIGHT_DOUBLE_ARROW 7
#define LEFT_ARROW 8

#define ZERO_IN_BRACKETS 9
#define ONE_IN_BRACKETS 10

#define DEFAULT_INPUT 0
#define DEFAULT_OUTPUT 1
#define DEFAULT_ERROR 2

#define REGULAR_CMD 0
#define FIRST_CONV_CMD 41
#define LAST_CONV_CMD 42

