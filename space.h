

#ifndef H_space
#define H_space

#include "VCB.h"

int freeSpace(int startingBlockNumber, int totalBlocks, int blockSize, unsigned char *bitMapPtr);
int loadSpace(int blockSize,unsigned char *bitMapPtr);
unsigned int allocateFreeSpace(int numOfBlocks, VCB *vcb,unsigned char *bitMapPtr);



#endif