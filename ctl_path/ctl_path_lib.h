#define SECTOR_SIZE 512
#define SECTOR_BIT 9
#define KEY_SIZE 16
#define MAC_SIZE (KEY_SIZE*2)
#define VERSION_SIZE 4
#define FD_SIZE 4

#define __NR_enc_rdafwr 333

#define P_SIZE 4096
#define K_SIZE 1024
#define IO_SIZE 8192
#define RESPONSE_SIZE SECTOR_SIZE
#define PAGE_BIT 12
#define NAME_LEN 16
#define NODE_SIZE 4096
//DS_param, ds_cmd는 개발에 따라 정책이 수정될 수 있다.
//fs_open.c와 openssd jasmine펌웨어 sata_table.c, sata.h 와 반드시 맞춰줄것.
typedef struct DS_param{
    unsigned int fd;
    unsigned char cmd;
    unsigned long offset; //여기가 LBA영역에 들어감 6bytes
    unsigned int size; //이건 lba처럼 count영역에 들어가니, 섹터단위일듯.
    unsigned int ret_time;
}DS_PARAM;

enum ds_cmd{
    DS_WR_RANGE_MIN = 0x43,
    DS_CREATE_WR = 0x44,
    DS_OPEN_WR = 0x45,
    DS_CLOSE_WR = 0x46,
    DS_REMOVE_WR = 0x47,
    DS_RITE_WR = 0x48,
    DS_WR_RANGE_MAX = 0x49,
    DS_RD_RANGE_MIN = 0x4A,
    DS_READ_RD = 0x4B,
    DS_AUTH_RD = 0x4C ,
    DS_CREATE_RD = 0x4D,
    DS_OPEN_RD = 0x4E,
    DS_CLOSE_RD = 0x4F,
    DS_REMOVE_RD =0x50,
    DS_WRITE_RD = 0x51,
    DS_RD_RANGE_MAX= 0x52
};

enum spm_cmd{
    SPM_CREATE = 0x65,
    SPM_CHANGE,
    SPM_DELETE,
    SPM_RECOVERY
};

typedef struct SPM_PARAM{
    int ret_time;
    int backup_cycle;
    int version_num;
    int cmd;
}spm_param;

int enc_rdafwr(DS_PARAM *ds_param, char* u_buf, char* response, int count);

int spm_send_cmd(int fd, char* buffer, int node_size, char* response, int pid, spm_param*sp);
