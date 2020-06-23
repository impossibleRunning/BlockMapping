#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "blkmap.h"
// if necessary, add other header files

extern FILE *devicefp;

extern FREEBLK freeblk ;
extern SpareData spare[BLOCKS_PER_DEVICE*PAGES_PER_BLOCK];
extern BlockMapTbl table;
extern FILE *devicefp;
extern int lbn;
extern int used_blk;
extern int erase ;

int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);


//
// flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
// 초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
// main()함수에서 반드시 먼저 호출이 되어야 한다.
//
void ftl_open()
{
	//
	// address mapping table 생성 및 초기화 등을 진행
    // mapping table에서 lbn과 pbn의 수는 blkmap.h에 정의되어 있는 DATABLKS_PER_DEVICE
    // 수와 같아야 하겠지요? 나머지 free block 하나는 overwrite 시에 사용하면 됩니다.
	// pbn 초기화의 경우, 첫 번째 write가 발생하기 전을 가정하므로 예를 들면, -1로 설정을
    // 하고, 그 이후 필요할 때마다 block을 하나씩 할당을 해 주면 됩니다. 어떤 순서대로 할당하는지는
    // 각자 알아서 판단하면 되는데, free block들을 어떻게 유지 관리할 지는 스스로 생각해 보기
    // 바랍니다.
	for (lbn = 0; lbn <= DATABLKS_PER_DEVICE; lbn++) {
		table.entry[lbn].pbn = -1;
	}
	for (int i = 0; i <= BLOCKS_PER_DEVICE * PAGES_PER_BLOCK - 1; i++) {
		spare[i].lsn = -1;
	}
	//set freeblk = -2
	for (int i = 0; i < PAGES_PER_BLOCK; i++) {
		spare[freeblk*PAGES_PER_BLOCK + i].lsn = -2;
	}
	return;
	
}

//
// file system이 ftl_write()를 호출하면 FTL은 flash memory에서 주어진 lsn과 관련있는
// 최신의 데이터(512B)를 읽어서 sectorbuf가 가리키는 곳에 저장한다.
// 이 함수를 호출하기 전에 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 한다.
// 즉, 이 함수에서 메모리를 할당받으면 안된다.
//
int ftl_read(int lsn, char *sectorbuf)
{
	lbn = lsn / PAGES_PER_BLOCK;
	int offset = lsn % PAGES_PER_BLOCK;
	int ppn = table.entry[lbn].pbn*PAGES_PER_BLOCK + offset;
	dd_read(ppn, sectorbuf);
	return ppn;
	
}

//
// file system이 ftl_write()를 호출하면 FTL은 flash memory에 sectorbuf가 가리키는 512B
// 데이터를 저장한다. 당연히 flash memory의 어떤 주소에 저장할 것인지는 block mapping 기법을
// 따라야한다.
//
int ftl_write(int lsn, char *sectorbuf)
{
	lbn = lsn / PAGES_PER_BLOCK;
	int offset = lsn % PAGES_PER_BLOCK;


	if (table.entry[lbn].pbn == -1) {
		table.entry[lbn].pbn = used_blk;
		used_blk++;
		spare[table.entry[lbn].pbn*PAGES_PER_BLOCK + offset].lsn = lsn;
		//spare[table.entry[lbn].pbn*PAGES_PER_BLOCK + offset].dummy = sectorbuf;
		dd_write(table.entry[lbn].pbn*PAGES_PER_BLOCK + offset, *sectorbuf);
		return table.entry[lbn].pbn*PAGES_PER_BLOCK + offset;
	}


	else if (table.entry[lbn].pbn != -1 &&
		spare[table.entry[lbn].pbn*PAGES_PER_BLOCK + offset].lsn == -1) {
		spare[table.entry[lbn].pbn*PAGES_PER_BLOCK + offset].lsn = lsn;
		dd_write(table.entry[lbn].pbn*PAGES_PER_BLOCK + offset, *sectorbuf);
		return table.entry[lbn].pbn*PAGES_PER_BLOCK + offset;
	}


	else if (table.entry[lbn].pbn != -1 &&
		spare[table.entry[lbn].pbn*PAGES_PER_BLOCK + offset].lsn != -1) {
		//erase operation.  how?
		for (int i = 0; i < PAGES_PER_BLOCK; i++) {
			spare[freeblk*PAGES_PER_BLOCK + i].lsn =
				spare[table.entry[lbn].pbn*PAGES_PER_BLOCK + i].lsn;   //copy to freeblk
			dd_write(freeblk*PAGES_PER_BLOCK + i, dd_read(table.entry[lbn].pbn*PAGES_PER_BLOCK + i, *sectorbuf));
			spare[table.entry[lbn].pbn*PAGES_PER_BLOCK + i].lsn = -2; //reset freeblk
		}
		dd_erase(table.entry[lbn].pbn);
		int blk_to_erase = table.entry[lbn].pbn;
		table.entry[lbn].pbn = freeblk;
		freeblk = blk_to_erase;
		erase++;
	}
	
}
void printTable() {
	for (lbn = 0; lbn <= DATABLKS_PER_DEVICE; lbn++) {
		printf("lbn:%d	pbn:%d\n", lbn, table.entry[lbn].pbn);
	}
	for (lbn = 0; lbn < BLOCKS_PER_DEVICE * PAGES_PER_BLOCK; lbn++) {
		printf("spare%d:	%d\n", lbn, spare[lbn].lsn);
	}
}
