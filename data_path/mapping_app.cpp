#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <map>

#define POLICY_LIST "/home/jeewon/Desktop/policy_list"
#define POLICY_MAP_FILE "/home/jeewon/Desktop/policy_file_mapping"
const int BUF_MAX_SIZE = 1000;
const int para_MAX_SIZE = 3;
const int para_MAX_LEN = 100; //same as max directory size



typedef struct packet{
    int flag;
    int data;
} _packet;

typedef struct packet_in{
    int flag;
    int pid;
    int fid;
} _packet_in;

int get_fileID(char*in){
    int len = 0;
    int ret = 0;
    for(; in[len] != '\0'; len++){
        ret += in[len];
    }
    return ret % 100;
}

int main() {
    FILE*fp = fopen(POLICY_LIST, "r+");
    if(fp == NULL){
        printf("There is no policy file\n");
        return 0;
    }
    
    int policy_cnt = 0;
    
    char policy_tb[10][3];
    int shmid_pid = shmget((key_t)0x1234, sizeof(_packet), IPC_CREAT | 0666);
    _packet*shmaddr_f_to_p = (_packet*)shmat(shmid_pid, NULL, 0);
    
    int shmid_fid = shmget((key_t)0x1235, sizeof(_packet), IPC_CREAT | 0666);
    _packet*shmaddr_p_to_f = (_packet*)shmat(shmid_fid, NULL, 0);
    
    int shmid_in = shmget((key_t)0x1236, sizeof(_packet_in), IPC_CREAT | 0666);
    _packet_in*shmaddr_in = (_packet_in*)shmat(shmid_in, NULL, 0);
    
    while(1){
        int eof, i = 0;
        char line[10];
        while(1){
            char tmp = 0 ;
            eof = fscanf(fp, "%c", &tmp);
            line[i++] = tmp;
            //printf("%c", tmp);
            if(tmp == '\n' || eof == EOF) break;
        }
        if(eof == EOF) break;
        
        policy_tb[policy_cnt][0] = line[0]-'0';
        policy_tb[policy_cnt][1] = line[2]-'0';
        policy_tb[policy_cnt][2] = line[4]-'0';
        
        policy_cnt++;
    }
    fclose(fp);
    
    std::map<int, int> m;
    fp = fopen(POLICY_MAP_FILE, "r");
    while(1){
        int eof, FID, PID;
        eof = fscanf(fp, "%d %d\n", &FID, &PID);
        if(eof == EOF) break;
        m[FID] = PID;
    }
    fclose(fp);
    
    printf("policy list\n");
    printf("pid   ret_time   backup_cycle\n");
    
    for(int i = 0; i < policy_cnt; i++){
        printf("  %d          %d              %d\n", policy_tb[i][0],policy_tb[i][1],policy_tb[i][2]);
    }
    
    while(1){
        int pid = 0, fileID = 0;
        
        if(shmaddr_in->flag){
            shmaddr_in->flag = 0;
            pid = shmaddr_in->pid;
            fileID = shmaddr_in->fid;
            printf("fileID pid %d %d\n", fileID, pid);
            m[fileID] = pid;
        }
        if(pid == -1 && fileID == -1) break;
        
        int fid_data;
        if(shmaddr_f_to_p->flag){
            fid_data = shmaddr_f_to_p->data;
            shmaddr_f_to_p->flag = 0;

            if(m.at(fid_data) == -1){
                printf("there is no fid like it\n");
                continue;
            }
            shmaddr_p_to_f->data = m[fid_data];
            shmaddr_p_to_f->flag = 1;
        }
    }
    
    fp = fopen(POLICY_MAP_FILE, "w");
    std::map<int, int>::iterator iter;
    for(iter = m.begin(); iter != m.end() ; iter++){
        fprintf(fp, "%d %d\n",iter->first, iter->second);
        printf("%d %d\n",iter->first, iter->second);
    }
    fclose(fp);
    
    return 0;
}
