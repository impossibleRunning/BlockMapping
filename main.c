#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "blkmap.h"

FILE *devicefp;

/****************  prototypes ****************/
void ftl_open();
int ftl_write(int lsn, char *sectorbuf);
int ftl_read(int lsn, char *sectorbuf);
void printTable();


SpareData spare[BLOCKS_PER_DEVICE*PAGES_PER_BLOCK];
BlockMapTbl table;
FILE *devicefp;
int lbn;
int used_blk = 0;
int erase = 0;
FREEBLK freeblk = BLOCKS_PER_DEVICE - 1;


//
// 이 함수는 file system의 역할을 수행한다고 생각하면 되고,
// file system이 flash memory로부터 512B씩 데이터를 저장하거나 데이터를 읽어 오기 위해서는
// 본인이 구현한 FTL의 ftl_write()와 ftl_read()를 호출하면 됨
//
int main(int argc, char *argv[])
{
	char *blockbuf;
    char sectorbuf[SECTOR_SIZE];
	int lsn, i;

    devicefp = fopen("flashmemory", "w+b");
	if(devicefp == NULL)
	{
		printf("file open error\n");
		exit(1);
	}
	   
    //
    // flash memory의 모든 바이트를 '0xff'로 초기화한다.
    // 
    blockbuf = (char *)malloc(BLOCK_SIZE);
	memset(blockbuf, 0xFF, BLOCK_SIZE);

	for(i = 0; i < BLOCKS_PER_DEVICE; i++)
	{
		fwrite(blockbuf, BLOCK_SIZE, 1, devicefp);
	}

	free(blockbuf);

	ftl_open();    // ftl_read(), ftl_write() 호출하기 전에 이 함수를 반드시 호출해야 함

	//
	// call ftl_write() functions for experimental evaluation 
    // using given pattern data files
	//
	
	printTable();
	printf("-------------------\n");
	for (int i = 0; i < DATABLKS_PER_DEVICE*PAGES_PER_BLOCK; i++) {

		ftl_write(i, sectorbuf[i]);
	}

	ftl_write(1, sectorbuf[i]);
	ftl_write(35, sectorbuf[i]);
	fclose(devicefp);

	return 0;
}
