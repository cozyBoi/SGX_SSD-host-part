#include <iostream>
#include <string.h>
#include "ctl_path_lib.h"

const int BUF_MAX_SIZE = 1000;
const int para_MAX_SIZE = 3;
const int para_MAX_LEN = 100; //same as max directory size

#define SPM_CREATE 0x5
#define SPM_CHANGE 0x6
#define SPM_DELETE 0x7

#define POLICY_LIST "/home/jeewon/Desktop/policy_list"

void line_input(char in[BUF_MAX_SIZE]){
    char buf = 0;
    for(int i = 0; 1; i++){
        scanf("%c", &buf);
        in[i] = buf;
        if(buf == '\n'){
            in[i] = 0;
            break;
        }
        else if(i == BUF_MAX_SIZE && buf != '\n'){
            //buffer size error
            fprintf(stderr, "[error] MAX buffer size is %d\n", BUF_MAX_SIZE);
        }
    }
}

void parse_str(char in[BUF_MAX_SIZE], char out[para_MAX_SIZE][para_MAX_LEN]){
    char tmp[BUF_MAX_SIZE];
    int len = 0, para_size = 0, j = 0;
    //assume no space allows in first and last "in[]" components
    for(int i = 0; i < BUF_MAX_SIZE && in[i] != 0; i++){
        while(in[i] == ' ') i++;
        if(len != 0) tmp[len++] = ' ';
        while(in[i] != ' ' && in[i] != 0) {
            tmp[len++] = in[i];
            i++;
        }
    }
    
    for(int i = 0; i < len; i++){
        j = 0;
        while(tmp[i] != ' ' && i < len){
            out[para_size][j++] = tmp[i++];
        }
        out[para_size++][j] = 0;
    }
}

int main() {
    printf("**************************************************************\n");
    printf("* SGX-SSD policy manager                                     *\n");
    printf("*                                                            *\n");
    printf("* press command and parameter                                *\n");
    printf("*                                                            *\n");
    printf("* ex) {create|change|delete} {ret_time} {Backup_cycle}       *\n");
    printf("*                                                            *\n");
    printf("* units:                                                     *\n");
    printf("* retention time : day                                       *\n");
    printf("* Backup cycle   : day                                       *\n");
    printf("**************************************************************\n");
    
    //assume aurora input
    char in[1000];
    line_input(in);
    //debug
    //printf("%s\n", in);
    
    char para_arr[para_MAX_SIZE][para_MAX_LEN]; //save parameters
    parse_str(in, para_arr);
    
    //debug
    
     printf("parameter lists : \n");
     for(int i = 0; i < para_MAX_SIZE; i++){
     printf("%s\n", para_arr[i]);
     }
    
    char path[100];
    int command, retention_time, backup_cycle, version_number, pid;
    
    int branch = 0;
    
    if(para_arr[0][0] == 'c' && para_arr[0][1] == 'r'){
        command = SPM_CREATE;
    }
    else if(para_arr[0][0] == 'c' && para_arr[0][1] == 'h'){
        command = SPM_CHANGE;
    }
    else if(para_arr[0][0] == 'd' || para_arr[0][0] == 'D'){
        command = SPM_DELETE;
    }
    else if(para_arr[0][0] == 'r' || para_arr[0][0] == 'R'){
        //no recovery
        command = 3;
        branch = 1;
    }
    else{
        fprintf(stderr, "error : command\n");
        return 0;
    }
    
    if(command == SPM_CHANGE){
        //what pid to change?
        printf("what pid to change : ");
        scanf("%d", &pid);
    }
    
    strcpy(path, para_arr[1]);
    
    if(!branch){
        //not recovery
        retention_time = atoi(para_arr[1]);
        backup_cycle = atoi(para_arr[2]);
        //version_number = atoi(para_arr[4]);
    }
    else{
        //recovery
        //??
    }
    //debug
    //printf("data : %d%s%d%d%d\n", command, path, retention_time, backup_cycle, version_number);
    printf("data : %d%d%d\n", command, retention_time, backup_cycle);
    
   // FILE*fp = fopen("policy_list", "r+");
    
    
    //***********
    //must change
    //***********
    FILE*fp = fopen(POLICY_LIST, "r+");
    
    //get one line
    
    int policy_cnt = 0;
    
    char policy_tb[10][3];
    
    while(1){
        //get command list
        /*
            contents :
            pid / ret / cycle
        */
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
        
        if(policy_tb[policy_cnt][1] == retention_time &&  policy_tb[policy_cnt][2] == backup_cycle && command == 0){
            printf("The policy already exist\n");
            fclose(fp);
            return 0;
        }
        policy_cnt++;
    }
    
    fclose(fp);
    
    //ssd에 cmd날린다.
    char buf[100];
    char resp[100];
    spm_param sp;
    sp.ret_time = retention_time;
    sp.backup_cycle = backup_cycle;
    sp.version_num = 0;
    sp.cmd = command;
    if(spm_send_cmd(0, buf, 0, resp, policy_cnt, &sp) == -1){
        printf("[spm] error command didn't reach to ssd");
        return 0;
    }
    
    //성공시 파일에 반영
    
    //fp = fopen("policy_list", "w");
    fp = fopen(POLICY_LIST, "w");
    
    for(int i = 0; i < policy_cnt; i++){
        if(command == 1 && policy_tb[policy_cnt][0] == pid){
            policy_tb[policy_cnt][0] = pid;
            policy_tb[policy_cnt][1] = retention_time;
            policy_tb[policy_cnt][2] = backup_cycle;
        }
        fprintf(fp, "%d %d %d\n", policy_tb[i][0], policy_tb[i][1], policy_tb[i][2]);
        printf("%d %d %d\n", policy_tb[i][0], policy_tb[i][1], policy_tb[i][2]);
    }
    
    
    if(command == 0){
        fprintf(fp, "%d %d %d\n", policy_cnt, retention_time, backup_cycle);
        printf("%d %d %d\n", policy_cnt, retention_time, backup_cycle);
    }
    
    fclose(fp);
    
    return 0;
}
