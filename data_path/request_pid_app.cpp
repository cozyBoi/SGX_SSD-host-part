#include <iostream>
#include <stddef.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
/* Not technically required, but needed on some UNIX distributions */
#include <sys/types.h>
#include <sys/stat.h>

typedef struct packet{
    int flag;
    int data;
} _packet;

int get_fileID(char*in){
    int len = 0;
    int ret = 0;
    for(; in[len] != '\0'; len++){
        ret += in[len];
    }
    return ret % 100;
}

size_t write(unsigned int fd, char*_buff, int size, int pid){
    char buff[100];
    strncpy(buff, _buff, size);
    strncpy(buff + size, (char*)&pid, 4);
    size_t ssize = size + 4;
    return syscall(write, fd, buff, ssize);
}

char*string = "hello world!";


int main() {
    int shmid_pid = shmget((key_t)0x1234, sizeof(_packet), IPC_CREAT | 0666);
    _packet*shmaddr_f_to_p = (_packet*)shmat(shmid_pid, NULL, 0);
    
    int shmid_fid = shmget((key_t)0x1235, sizeof(_packet), IPC_CREAT | 0666);
    _packet*shmaddr_p_to_f = (_packet*)shmat(shmid_fid, NULL, 0);
    
    char*in = "file0";
    shmaddr_f_to_p->data = get_fileID(in);
    shmaddr_f_to_p->flag = 1;
    shmaddr_p_to_f->flag = 0;
    while(!shmaddr_p_to_f->flag);
    int pid = shmaddr_p_to_f->data;
    printf("pid : %d\n", pid);
    
    int fd = open("file0", O_WRONLY | O_APPEND);
    
    write(fd, string, strlen(string), pid);
    
    return 0;
}
