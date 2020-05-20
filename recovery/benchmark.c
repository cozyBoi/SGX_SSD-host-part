#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>

//#include "frecovery.h"

#define OPEN_KEY 333
#define CLOSE_KEY 334
#define READ_KEY 335
#define WRITE_KEY 336
#define MAX 1024

int main(int *argc, char **ayrgv)
{
    int fd;
    int fd_key;
    int key;
    int close;
    int readn = 0;
    int writen = 0;
    char bufR[MAX];
    char *bufW = "fklsdjfklsjdfkldsjflkjsdlkf";
    int i=0;
    char file_path[3][20] = {"/SSD/foo7.txt", "/SSD/foo8.txt", "/SSD/foo9.txt"};
	char bufS[512];
    key= 0x11223344;
    //int fd = fopen("??");
	//frecovery(fd, 3);

    return 0;


}
