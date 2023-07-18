/**************************************************************
* Class:  CSC-415-02 Summer 2023
* Names: Saripalli Hruthika, Nixxy Dewalt, Alekya Bairaboina, Banting Lin 
* Student IDs: 923066687, 922018328, 923041428, 922404012
* GitHub Name: hru952
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
#include "fsFunc.h"

#define magicNum 0x21736569626D6F5A //translates to "Zombies!" in hexdump

// Function to initialize the free space bitmap
int freeSpace(int startingBlockNumber, int totalBlocks, int blockSize)
{

    int bitmapSize = (5 * blockSize); //Calculate the bitmap size based on the block size
    bitMapPtr = malloc(bitmapSize); // Allocate memory for bitmap

    if (bitMapPtr == NULL)
    {
        printf("Failed to allocate memory for bitmap in freeSpace\n");
        return (-1);
    }

    // Setting each byte to 0x00 so that all blocks are initially free.
    for (int i = 0; i < bitmapSize; i++)
    {
        bitMapPtr[i] = 0x00;
    }

    //Set bitMap to 0xFC (11111100 in binary) so that bit 0 (VCB start block) and 
    //Bits 1-5(1-5 blocks for bitmap) are not free and not available for allocation 
    bitMapPtr[0] = 0xFC; 
  
    printf("\nFree space initialization completed successfully.\n");

    return startingBlockNumber; // location of free space bit map
}



//Function to initialize root directory
unsigned int rootDir(int numOfDirEnt, char* flag, DE* parent) 
{

/*Decide how many Directory Entries (DE) you want for a directory */
    DE * dirEnt[numOfDirEnt]; 

// Now multiply the size of your directory entry by the number of entries./
    unsigned long dirSize = numOfDirEnt * sizeof(DE); 

// Now you need to determine how many blocks you need./
    int blocksForDir = (dirSize + (vcb->blockSize - 1)) / (vcb->blockSize);
    int realBytes = blocksForDir * vcb->blockSize;
    int real_numOfDirEnt = realBytes / sizeof(DE);
    unsigned long realSize = real_numOfDirEnt * sizeof(DE);

/*Now you have a pointer to an array of directory entries. Loop through them 
  and initialize each directory entry structure to be in a known free state.*/
    printf("\nInitializing Directory structure to known free state\n");
    for (int i = 0; i < numOfDirEnt; i++)
    {
        dirEnt[i] = malloc(sizeof(DE)); 
        if(i>1)
	    {
		dirEnt[i]->fileName[0] = '\0';
	    }
    }
// Now ask the free space system for 'blocksForDir' blocks/
    unsigned int location = allocateFreeSpace(blocksForDir);
    if (location == -1) {
        printf("\nFailed to allocate free blocks for root directory\n");
        return(-1);
    }
    printf("\nTotal blocks for Root Directory = %d and size is = %ld\n",blocksForDir,realSize);
    printf("\nRoot Directory block number = %d\n", location);

// Set 1st Directory entry '.'
    if(parent == NULL)
    {
        dirEnt[0]->location = location;
        strcpy(dirEnt[0]->fileName, ".");
    	dirEnt[0]->fileSize = (realSize); 
    	strcpy(dirEnt[0]->fileType, flag);  
    	dirEnt[0]->dirBlocks = blocksForDir;
    	time(&(dirEnt[0]->created));
    	time(&(dirEnt[0]->lastModified));
    }
    else
    {
	dirEnt[0] = parent;
    }
    
    printf("\n1st Directory Entry set\n");

// Set 2nd Directory entry '..'
    dirEnt[1]->location = location;
    strcpy(dirEnt[1]->fileName, "..");
    dirEnt[1]->fileSize = (realSize);  
    strcpy(dirEnt[1]->fileType, flag);
    dirEnt[1]->dirBlocks = blocksForDir;
    time(&(dirEnt[1]->created));
    time(&(dirEnt[1]->lastModified));
    printf("\n2nd Directory Entry set\n");

// Now write the root directory

    char *buff = malloc(blocksForDir * vcb->blockSize);
    char *buffLocation = buff;
    for (int i = 0; i < numOfDirEnt; i++)
    {
        memcpy(buffLocation, dirEnt[i], sizeof(DE));
        buffLocation += sizeof(DE);
    }

    if(!(LBAwrite(buff, blocksForDir, 6))){ 
        printf("\nError: Root Directory writing failed\n");
    }
    printf("\nRoot Directory written to disk\n");
    
    free(buff); //free buffer
    buff = NULL;

// Return the starting block number of the root directory/
    return location;
}

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
    printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

    //globals for use of blocksize and numberof blocks
    //blocksInVolume = numberOfBlocks;
    //sizeOfBlock = blockSize;
    //totDirEnt = 30;
    //Malloc a Block of memory as your VCB pointer and read block 0 i.e VCB block
    vcb = malloc(blockSize);
    LBAread(vcb,1,0);

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
	unsigned int rootBlock = rootDir(totDirEnt,"d",NULL);
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

