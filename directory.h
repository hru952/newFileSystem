


#ifndef H_directory
#define H_directory


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "VCB.h"
#include "fsLow.h"
#include "mfs.h"


//extends





struct Extent {
    unsigned int loc; // Starting location of the extent
    unsigned int count; // Total used blocks in the extent

};

typedef struct dirEntry
{
	char fileName[100];//Name of file        
	unsigned int location; //Starting location of the directory blocks
        unsigned long fileSize; //Size of the blob
        char fileType[1];//Flag to identify directory or normal blob
        int dirBlocks;//Number of blocks it occupies
        time_t created; //Created time
        time_t lastModified;//Last modified time     
} DE;



unsigned int rootDir(int numOfDirEnt, DE* parent, VCB *vcb,unsigned char *bitMapPtr);

#endif