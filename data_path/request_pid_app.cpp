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

#define FILE_PATH "/home/jeewon/Desktop/file0"
#define SECTOR_SIZE 512
#define IO_SIZE 4096
#define LIMIT_SIZE 8192
#define SECTOR_BIT 9

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

/*
size_t _write(unsigned int fd, char*_buff, int size, int pid){
    char buff[100];
    strncpy(buff, _buff, size);
    strncpy(buff + size, (char*)&pid, 4);
    size_t ssize = size + 4;
    return write(fd, buff, ssize);
}*/

size_t _write(unsigned int fd, char*_buff, int size, int pid){
    //maximum IO amount -> 4K
    char *u_buf_ = (char*) malloc (sizeof(char)*(IO_SIZE + SECTOR_SIZE));    //IO_SIZE??
    char *u_buf = (char*) ((((unsigned long)u_buf_ + SECTOR_SIZE -1 ) >> SECTOR_BIT) << SECTOR_BIT);
    
    int chunk_num = 0;
    if(IO_SIZE < size){
        chunk_num = size / IO_SIZE;
    }
    char*tag = "9999";
    for(int i = 0; i <= chunk_num; i++){
        //split each file in 4K and piggy back pid && tag
        memcpy((char*)u_buf, _buff + i * IO_SIZE, IO_SIZE);
        memcpy((char*)u_buf + IO_SIZE, (char*)&pid, 4);
        memcpy((char*)u_buf + IO_SIZE + 4, tag, 4);
        write(fd, u_buf, IO_SIZE + 4 + 4);
    }
    
    return 0;
    /*
    for(int i = 0; i < chunk_num; i++){
        //split file to 4K and piggy back pid
        memcpy((char*)u_buf, _buff + i * IO_SIZE, IO_SIZE);
        memcpy((char*)u_buf + IO_SIZE, (char*)&pid, 4);
        write(fd, u_buf, IO_SIZE + 4);
    }
    
    //last chunk handling
    int last_chunk_size = size % IO_SIZE;
    if(last_chunk_size < 512){
        //512로 바꿔주기
        last_chunk_size = last_chunk_size + SECTOR_SIZE % last_chunk_size;
    }
    else{
        //512의 배수로 맞춰주기
        last_chunk_size = last_chunk_size + last_chunk_size % SECTOR_SIZE;
    }
    
    memcpy((char*)u_buf, _buff + chunk_num * IO_SIZE, last_chunk_size);
    memcpy((char*)u_buf + last_chunk_size, (char*)&pid, 4);
    
    return write(fd, u_buf, last_chunk_size + 4);
     */
}

char*string = "hello world!";


int main() {
    int shmid_pid = shmget((key_t)0x1234, sizeof(_packet), IPC_CREAT | 0666);
    _packet*shmaddr_f_to_p = (_packet*)shmat(shmid_pid, NULL, 0);
    
    int shmid_fid = shmget((key_t)0x1235, sizeof(_packet), IPC_CREAT | 0666);
    _packet*shmaddr_p_to_f = (_packet*)shmat(shmid_fid, NULL, 0);
    
    char*in = FILE_PATH;
    shmaddr_f_to_p->data = get_fileID(in);
    shmaddr_f_to_p->flag = 1;
    shmaddr_p_to_f->flag = 0;
    while(!shmaddr_p_to_f->flag);
    int pid = shmaddr_p_to_f->data;
    printf("pid : %d\n", pid);
    
    int fd = open(FILE_PATH, O_WRONLY | O_APPEND);
    
    _write(fd, string, strlen(string), pid);
    
    return 0;
}
