#ifndef	_BLOCK_MAPPING_H_
#define	_BLOCK_MAPPING_H_

#define TRUE				1
#define	FALSE				0

#define	SECTOR_SIZE			512			
#define	SPARE_SIZE			16			
#define	PAGE_SIZE			(SECTOR_SIZE+SPARE_SIZE)
#define SECTORS_PER_PAGE	1
#define	PAGES_PER_BLOCK		4  // updatable
#define	BLOCK_SIZE			(PAGE_SIZE*PAGES_PER_BLOCK)
#define	BLOCKS_PER_DEVICE	32 // updatable
#define DATABLKS_PER_DEVICE	(BLOCKS_PER_DEVICE - 1)	// 한 개의 free block을 유지
typedef int FREEBLK;
//
// If necessary, constants can be added
// 


typedef struct
{
	int lsn;									// lsn means what logical page occupies this physical page
	char dummy[SPARE_SIZE - 4];
} SpareData;

typedef struct
{
//	int lbn;									// not necessary
	int pbn;
//	int first_free_page_offset;					// not necessary
} BlkMapTblEntry;

typedef struct									// block mapping table
{
	BlkMapTblEntry entry[DATABLKS_PER_DEVICE];
} BlockMapTbl;

//
// If necessary, you can add new data types or modify the upper type definitions.
// This header file is used just for reference. 
//


#endif
