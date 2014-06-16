#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MSG_SIZE 100
#define MAX_CLIENTS 100
#define MYIP "127.0.0.1"
#define MYPORT 7400
#define MYPATH "log.txt"