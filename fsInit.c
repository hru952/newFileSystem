/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
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

typedef struct VCB
{
    long signature;               // Unique number to validate the VCB
    int totalBlocks;              // To store total number of blocks
    int blockSize;                // To store size of a block
    unsigned int rootLocation;    // Location of root node
    unsigned int bitMapLocation;  // Starting location of the bitmap to track free blocks
} VCB;

unsigned char *bitMapPtr; // to hold the memory address of the free space bitmap.
VCB *vcb; //to hold address of the vcb
// initializes the free space bitmap
int freeSpace(int startingBlockNumber, int totalBlocks, int blockSize)
{

    printf("FREEE SPACE CALLEEEEEEEEEEEEEEEEEEEEEEEEEEEEED");
    int bitmapSize = (5 * blockSize); //Calculate the bitmap size based on the block size
    bitMapPtr = malloc(bitmapSize); // Allocate memory for bitmap

    if (bitMapPtr == NULL)
    {
        printf("Failed to allocate memory for bitmap in freeSpace\n");
        return (-1);
    }

   
    // Setting each byte to 0x00 so that all blocks are initially free and available for allocation.
    for (int i = 0; i < bitmapSize; i++)
    {
        bitMapPtr[i] = 0x00;
    }

        /*alekya
        //VCB vcb; // Create an instance of the VCB struct
        // vcb.bitMapLocation = startingBlockNumber;
        */

    //Setting bitMapPtr[0] to 0xFC (11111100 in binary) so that bit 0 (VCB start block) and 
    // Bits 1-5(1-5 blocks) are not free and not available for allocation 
    bitMapPtr[0] = 0xFC; 
  
    printf("Free space initialization completed successfully.\n");

    return startingBlockNumber; // location of free space bit map
}

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
    printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
    //check if vcb exists aka check signature

    //if vcb exits load it up

    // if vcb does not exist create it

    //creating vcb

    {    // Create an instance of the VCB struct
        vcb = malloc(sizeof(VCB)); 
        //we want to create a bitmap starting at location 1, not 0 since 0 represents the vcb
        // bitmap will occupy blocks 1-5
        // vcb will occupy block 0;
         //call free space
        vcb->bitMapLocation = freeSpace(1,numberOfBlocks, blockSize);
        //assign signature dummy value for the time being
        vcb->signature = 12345678;
        vcb->blockSize = blockSize;
        vcb->totalBlocks = numberOfBlocks;
   
        //assign root node
        //assign this to the pointer
    }


    /* TODO: Add any code you need to initialize your file system. */

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
