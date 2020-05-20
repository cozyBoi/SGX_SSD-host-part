/*
fibmap 참조, 특히 hdparm 코드
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <assert.h>
#include <linux/hdreg.h>
#include <dirent.h>
#include <sys/sysmacros.h>

#include "frecovery.h"

//#define PATH_MAX 30
#define SECTOR_SIZE 512
#define BUF_SIZE 16384
#define BUF_ELE_SIZE 12
#define BUF_LBA_SIZE 8

#define RECOVERY_KEY 337

static unsigned long get_start_lba(dev_t dev);
static void find_dev_file(dev_t dev, char *dev_file);
static void print_buf(char *buf, unsigned int buf_size);

static unsigned char lba_list[BUF_SIZE];


//entry point of this c file
//main -> frecovery(fd_key, 3);
int frecovery(int fd, unsigned int recovery_time)
{
	unsigned int blkcnt;
	unsigned int blocksize;
	unsigned int sec_per_blk;
	struct stat st;
	unsigned int i, block;
	unsigned int block_bf;
	unsigned long start_lba;	
	unsigned long lba;
	unsigned int size;
	unsigned int lba_list_size;
	//unsigned char lba_list[BUF_SIZE];

	printf("frecovery (%d %d)\n", fd, recovery_time);

	if (ioctl(fd, FIGETBSZ, &blocksize))
	{
		printf("ioctl FIBGETBSZ error\n");
	}
	if (fstat(fd, &st)) //fstat -> return file information
	{
		printf("fstat error\n");
	}

//	printf("file size: %d\n", st.st_size);
    
    //st_size -> whole size of file
    //get number of blk, 파일의 크기가 블락을 하나 더 더해야하는거 같은데 왜 그러지 -> 아 디스크 실드는 아마 키를 피기백할 거임
	blkcnt = (st.st_size + blocksize - 1) / blocksize;
	sec_per_blk = blocksize/SECTOR_SIZE;
	//printf("File size %d blocks %d blocksize %d\n", (int)st.st_size, blkcnt, blocksize);

	//printf("check major: %d %d\n", major(st.st_dev), minor(st.st_dev));
	//get start lba
//	get_dev_t_geometry (st.st_dev, NULL, NULL, NULL, &start_lba, NULL, &sector_bytes);
    
    //st_dev -> ID of dev containing the file
	start_lba=get_start_lba(st.st_dev);
	printf("start lba: %lu\n", start_lba);

	//get lba list
	block_bf=0xfffffffe; //block before
	size=0;
	lba_list_size=0;
	for (i = 0; i < blkcnt; i++) {
		block = i;
		//printf("offset: %d\n", lseek(fd, 0, SEEK_CUR));
        //n번째 블락의 시스템 블락 넘버를 가져오는 ioctl
		if (ioctl(fd, FIBMAP, &block)) {
			perror("FIBMAP ioctl failed");
		}
		if(block_bf+1 != block)
		{
            //core -> 연속적이면 걍 더하다가 연속적이지 않을때 여기에 들어오는데 전의 연속적인거를 다 LBA에 저장하고, 다음 블락으로 가는거
            //예를 들어 0 1 2 번 블락의 시스템 블락넘버를 ioctl에서 갖고왔는데 연속적임 (5 6 7로). 근데 3번째꺼가 9면은 여기 들어오는거
			//printf("Not Contiguous!!!!!!!\n");
            //시작 + n-1개의 섹터 - (n개의 섹터 - 한개의 섹터) --> ??
			lba = start_lba + block_bf*sec_per_blk - (size-sec_per_blk);
			printf("LBA/size: %lu, %u\n", lba, size);
			//printf("lba_listn_idx: %d\n", lba_list_size);
			//sprintf(&(lba_list[lba_list_size]), "%lu%u", lba, size);
			*(unsigned long*)(&lba_list[lba_list_size]) = lba;
			*(unsigned int*)(&lba_list[lba_list_size+BUF_LBA_SIZE]) = size;
			lba_list_size+=BUF_ELE_SIZE;
			size = 0;
		}
		block_bf=block;
		size+=sec_per_blk; //블락마다의 섹터를 더하네
	//	printf("%3d %10lu\n", i, start_lba+block*sec_per_blk);	
	}	
	lba = start_lba + block*sec_per_blk -(size-sec_per_blk);
	printf("LBA/size: %lu, %u\n", lba, size);
	*(unsigned long*)(&lba_list[lba_list_size]) = lba;
	*(unsigned int*)(&lba_list[lba_list_size+BUF_LBA_SIZE]) = size;
	lba_list_size+=BUF_ELE_SIZE;			
	//sprintf(&lba_list[lba_list_size], "%lu%u", lba, size);
			
	print_buf(lba_list, lba_list_size);
    
    
    
	//send lba_listfer to the system call
	syscall(RECOVERY_KEY, fd, lba_list, lba_list_size, recovery_time);
}

static unsigned long get_start_lba(dev_t dev)
{
	unsigned long start_lba;
	//get start_lba
	struct hd_geometry{
		unsigned char	heads;
      unsigned char	sectors;
      unsigned short	cylinders;
      unsigned long	start;
	};

	//char dev_file[PATH_MAX]="/dev/sda4";
	char dev_file[PATH_MAX];
	//get device file name
	find_dev_file(dev, dev_file);
	printf("Device file in %s\n", dev_file);

	int fd = open(dev_file, O_RDONLY|O_NONBLOCK); 
	if(fd==-1)
	{
		printf("device file open error\n");
	}


        
	struct hd_geometry geom;
    //HDIO_GETGEO -> get device geometry
    //----------------------------------
    //heads    number of heads
    //sectors    number of sectors/track
    //cylinders    number of cylinders, mod 65536
    //start    starting sector of this partition.
    //----------------------------------
    
	ioctl(fd, HDIO_GETGEO, &geom);
	start_lba = geom.start;

	close(fd);

	return start_lba;
}

static void find_dev_file(dev_t dev, char *dev_file)
{
	DIR *dp;
	struct dirent *entry;
	char *dev_dir="/dev";
	unsigned int major_ = major(dev), minor_ = minor(dev);
	

	*dev_file = '\0';
	if(!(dp = opendir(dev_dir)))
	{
		printf("device directory open error\n");
	}
	
	while((entry = readdir(dp)) != NULL)
	{
		if(entry->d_type == DT_UNKNOWN || entry->d_type == DT_BLK)
		{
			struct stat st;
			sprintf(dev_file, "%s/%s", dev_dir, entry->d_name);
			//printf("%s\n", dev_file);
            //블락 디바이스인지 확인하는거 같음
			if(!stat(dev_file, &st) && S_ISBLK(st.st_mode))//S_ISBLK : Block device file
			{
			//	printf("%s: %d(%d), %d(%d)\n", dev_file, major_, minor_, major(st.st_rdev), minor(st.st_rdev));
				if((major_==(unsigned)major(st.st_rdev)) && (minor_ == (unsigned)minor(st.st_rdev)))
				{
                    //메이저 마이너 넘버를 확인해서 실제 그 디바이스의 dev파일인지 확인하는거같음
                    //major_, minor_ 에는 디바이스의 넘버가 들감
					printf("hihi\n");
					closedir(dp);
					return;
				}
			}
		}
	}
}

static void print_buf(char *buf, unsigned int buf_size)
{
	int i,j;

	printf("lba_list_size: %u\n", buf_size);
	for (i=0; i<buf_size; i+=12)
	{
		//for(j=0; j<8; j++)
			printf("%lu %u", *(unsigned long*)(&lba_list[i]), *(unsigned int*)(&buf[i+8]));
		printf("\n");
	}
}
