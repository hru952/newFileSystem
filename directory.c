
#include "directory.h"
#include "VCB.h"
#include "space.h"






//Function to initialize root directory
unsigned int rootDir(int numOfDirEnt, DE* parent, VCB *vcb, unsigned char *bitMapPtr) 
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
    unsigned int location = allocateFreeSpace(blocksForDir, vcb,bitMapPtr );
    if (location == -1) {
        printf("\nFailed to allocate free blocks for root directory\n");
        return(-1);
    }
    printf("\nTotal blocks for Root Directory = %d and size is = %ld\n",blocksForDir,realSize);
    printf("\nRoot Directory block number = %d\n", location);

// Set 1st Directory entry '.'
    dirEnt[0]->location = location;
    strcpy(dirEnt[0]->fileName, ".");
    dirEnt[0]->fileSize = (realSize); 
    strcpy(dirEnt[0]->fileType, "d");  
    dirEnt[0]->dirBlocks = blocksForDir;
    time(&(dirEnt[0]->created));
    time(&(dirEnt[0]->lastModified));
    printf("\n1st Directory Entry set\n");

// Set 2nd Directory entry '..'
    dirEnt[1]->location = location;
    strcpy(dirEnt[1]->fileName, "..");
    dirEnt[1]->fileSize = (realSize);  
    strcpy(dirEnt[1]->fileType, "d");
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