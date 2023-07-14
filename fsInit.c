/**************************************************************
* Class:  CSC-415-02 Summer 2023
* Names: Saripalli Hruthika, Nixxy Dewalt, Alekya Bairaboina, Banting Lin 
* Student IDs: 923066687, 922018328, 923041428, 922404012
* GitHub Name: hru952, Alekhya1311, Bentosboxs, tdragon00
* Group Name: Zombies
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "directory.h"
#include "VCB.h"
#include "space.h"

#define magicNum 0x21736569626D6F5A //translates to "Zombies!" in hexdump



unsigned char *bitMapPtr; // to hold the memory address of the free space bitmap.
VCB *vcb; //to hold address of the vcb


int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
    printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
    //Malloc a Block of memory as your VCB pointer and read block 0 i.e VCB block
    vcb = malloc(blockSize);
    LBAread(vcb,1,0);

    //check to see if signature is good 
    printf("signature is %ld \n",vcb->signature);

    //If signature matches, volume is alredy initialized.So load freespace bitmap.
    if( vcb->signature == magicNum)
    {
         if(loadSpace(blockSize))
        {
            printf("Free space loaded \n");
        }
        printf("signature is valid no need to create a new bitmap or vcb \n");
    }
    //If signature doesn't match, initialize freespace volume.
    else
    {
         printf("\nsignature is invalid overwriting hardrive data \n");
        
        //call the function that initializes freespace
        vcb->bitMapLocation = freeSpace(1,numberOfBlocks, blockSize);
        //Initialize all vcb values
        vcb->signature = magicNum; //vcb signature
        vcb->blockSize = blockSize;//block size
        vcb->totalBlocks = numberOfBlocks;//total blocks count
	//Call function to initialize root directory and 
	//initialize return value(location) to rootLocation in VCB.
	unsigned int rootBlock = rootDir(30, NULL, vcb);
        if (rootBlock == -1){
	    printf("\nError with root allocation\n");
            return(-1);
	}
        vcb->rootLocation = rootBlock;
    }
    return 0;
}

void exitFileSystem()
{
    // Write freeSpace to disk.
    //changed to zero from -1
    //I looked at LBA write and it seemes the return value for failure was 0
    if (LBAwrite(bitMapPtr, 5, 1) == 0)
    {
        printf("Failed to write the bitmap to disk\n");
    }

    if (LBAwrite(vcb, 1, 0) == 0)
    {
        printf("Failed to write the volume control block to disk\n");
    }

    // Free resources
    free(bitMapPtr);
    free(vcb);
    bitMapPtr = NULL;
    printf("System exiting\n");
}
