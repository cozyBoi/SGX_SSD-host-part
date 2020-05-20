#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>

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
    printf("pid : %d\n", shmaddr_p_to_f->data);
    
    return 0;
}
