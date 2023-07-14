#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "space.h"
#include "fsLow.h"
#include "VCB.h"

// Function to initialize the free space bitmap
int freeSpace(int startingBlockNumber, int totalBlocks, int blockSize,  unsigned char *bitMapPtr)
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

//Function to load bitmap to memory if signature matches
int loadSpace(int blockSize ,unsigned char *bitMapPtr)
{
    int bitmapSize = (5 * blockSize); //Calculate the bitmap size based on the block size
    bitMapPtr = malloc(bitmapSize); // Allocate memory for bitmap
    if (bitMapPtr == NULL)
    {
        printf("Failed to allocate memory for bitmap in freeSpace\n");
        return (-1);
    }

    //loading up the bitmap
    LBAread(bitMapPtr,5,1);
    return 1;
}

//Function to allocate required number of contiguous free blocks
unsigned int allocateFreeSpace(int numOfBlocks, VCB *vcb, unsigned char *bitMapPtr)
{
    // Hard coded starting block to 6 as first block 0=> VCB, Blocks 1 - 5 for bitmap
    unsigned int startingBlock = 6; 
    unsigned int consecutiveBlocks = 0; // Number of consecutive free blocks found
    unsigned int allocatedBlocks = 0; // Starting block index of the allocated space
    unsigned int blockNumber = startingBlock; // Current block being checked

    while (blockNumber < vcb->totalBlocks)
    {
        if (consecutiveBlocks == numOfBlocks)
        {
            //If required no: of contiguous free blocks found, break the loop
            allocatedBlocks = blockNumber - numOfBlocks;
            break;
        }
	// Get the byte from the bitmap corresponding to the block
        unsigned char temp = bitMapPtr[blockNumber / 8]; 
        int bitPosition = blockNumber % 8; //Get the bit position within the byte

        // Check if the current block is free (bit is 0)
        // If we have a byte temp with a value of 0b10101010 and we want to check 
        //the status of the 3rd bit (bit position 2).
        // The bitPosition variable would be set to 2, indicating the 3rd bit. 
        //The expression temp >> (7 - bitPosition) would become temp >> (7 - 2),
	//which simplifies to temp >> 5. 
	//Shifting the bits of temp 5 positions to the right, we get 0b00000101. 
        // The expression ((temp >> (7 - bitPosition)) & 1) would then be 
	//(0b00000101 & 0b00000001), which equals 0b00000001. 
        // The result is 1, indicating that the 3rd bit in temp is set to 1, 
	//which means the corresponding block is used (not free).

        if (((temp >> (7 - bitPosition)) & 1) == 0)
        {
            if (consecutiveBlocks == 0)
            {
                // If it's the first free block encountered, 
	        //update the starting block index of the allocated space
                allocatedBlocks = blockNumber;
            }

            consecutiveBlocks++; //Increment count of consecutive free blocks
        }
        else
        {
            consecutiveBlocks = 0; //Reset the count if a used block is encountered
        }

        blockNumber++; // Move to the next block
    }

    if (consecutiveBlocks < numOfBlocks)
    {
        // Unable to allocate the required number of blocks, return -1
        return -1;
    }

    // Mark the allocated blocks as used in the bitmap
    for (unsigned int i = allocatedBlocks; i < allocatedBlocks + numOfBlocks; i++)
    {
        unsigned int byteIndex = i / 8; // Get the byte index in the bitmap
        unsigned int bitIndex = i % 8; // Get the bit index within the byte
	// Set the corresponding bit to 1 to mark it as used
        bitMapPtr[byteIndex] |= (1 << (7 - bitIndex));
    }
    // Return the starting block index of the allocated space
    return allocatedBlocks; 
}


