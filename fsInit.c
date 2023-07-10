/**************************************************************
* Class:  CSC-415-02 Summer 2023
* Names: Saripalli Hruthika
* Student IDs: 923066687
* GitHub Name:
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

#define BLOCK_SIZE 512

typedef struct VCB
{
    long signature;               // Unique number to validate the VCB
    int totalBlocks;              // To store total number of blocks
    int blockSize;                // To store size of a block
    unsigned int rootLocation;    // Location of root node
    unsigned int bitMapLocation;  // Starting location of the bitmap to track free blocks
} VCB;

typedef struct dirEntry
{
        unsigned int location; //Starting location of the directory blocks
        char fileName[256];//Name of file
        unsigned long fileSize; //Size of the blob
        char fileType;//Flag to identify directory or normal blob
        int dirBlocks;//Number of blocks it occupies
        time_t created; //Created time
        time_t lastModified;//Last modified time
} DE;

DE * dirEnt[50]; //50 entries


unsigned char *bitMapPtr; // to hold the memory address of the free space bitmap.
VCB *vcb; //to hold address of the vcb
// initializes the free space bitmap

int freeSpace(int startingBlockNumber, int totalBlocks, int blockSize)
{

    //printf("FREEE SPACE CALLEEEEEEEEEEEEEEEEEEEEEEEEEEEEED");
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



int rootDir(char *name, char *type, DE * dirEntry, DE** parent) 
{

/*Decide how many Directory Entries (DE) you want for a directory */
    int numOfDirEnt = 50;

/*Now multiply the size of your directory entry by the number of entries.*/
    unsigned long dirSize = numOfDirEnt * sizeof(DE); 

/*Now you need to determine how many blocks you need.*/
    int blocksForDir = (dirSize + (vcb->blockSize - 1)) / (vcb->blockSize);

/*Now you have a pointer to an array of directory entries. Loop through them 
  and initialize each directory entry structure to be in a known free state.*/
    for (int i = 0; i < numOfDirEnt; i++)
    {
        dirEnt[i] = malloc(sizeof(DE)); 
    }
	
/*Now ask the free space system for 'blocksForDir' blocks*/
/*Note : Hard coded starting block to 6 as first block 0=> VCB, Blocks 1 - 5 for bitmap*/
    unsigned int location = freeSpace(6, blocksForDir, BLOCK_SIZE);


/*Set 1st Directory entry '.'*/
    dirEnt[0]->location = location;
    strcpy(dirEnt[0]->fileName, ".");
    dirEnt[0]->fileSize = (dirSize); 
    dirEnt[0]->fileType = "d";  
    dirEnt[0]->dirBlocks = blocksForDir;
    time(&(dirEnt[0]->created));
    time(&(dirEnt[0]->lastModified));

/*Set 2nd Directory entry '..'*/
    dirEnt[1]->location = location;
    strcpy(dirEnt[1]->fileName, "..");
    dirEnt[1]->fileSize = (dirSize);  
    dirEnt[1]->fileType = "d";
    dirEnt[1]->dirBlocks = blocksForDir;
    time(&(dirEnt[1]->created));
    time(&(dirEnt[1]->lastModified));

/*Now write the root directory*/

    //Need to write this block

/*Return the starting block number of the root directory*/
    return location;
}

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
    printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
    //check if vcb exists aka check signature
    //there has to be a better way of doing this 
    void *buffer = malloc(blockSize);


    //load up data where vcb would be
    LBAread(buffer,1,0);


    //we are casting this as a different pointer of different size.
    // while buffer is larger than vcb the data should be the same
    //I cannot read directly into vcb as vcb is smaller than blocksize most
    vcb = buffer;

    //check to see if signature is good 
      printf("signature is %ld \n",vcb->signature);
    if( vcb->signature == 12345678)
    {
        printf("signature is valid no need to create a new bitmap or vcb \n");

    }
    else
    {
         printf("signature is invalid overwriting hardrive data \n");
        // if vcb does not exist create it
       // Create an instance of the VCB struct

       // commenting out the malloc part need to re think about malloc
        //vcb = malloc(sizeof(VCB)); 

        
        //we want to create a bitmap starting at location 1, not 0 since 0 represents the vcb
        // bitmap will occupy blocks 1-5
        // vcb will occupy block 0;
         //call free space
        vcb->bitMapLocation = freeSpace(1,numberOfBlocks, blockSize);
        //assign signature dummy value for the time being hard coded for the time being
        vcb->signature = 12345678;
        vcb->blockSize = blockSize;
        vcb->totalBlocks = numberOfBlocks;
	vcb->rootLocation = rootDir("root", "d", NULL, NULL);
   
        //assign root node
        //assign this to the pointer
    }


    //free(buffer);
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
