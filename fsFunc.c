/**************************************************************
 * Class:  CSC-415-02 Summer 2023
 * Names: Saripalli Hruthika, Nixxy Dewalt, Alekya Bairaboina, Banting Lin
 * Student IDs: 923066687, 922018328, 923041428, 922404012
 * GitHub Name: hru952
 * Group Name: Zombies
 * Project: Basic File System
 *
 * File: fsFunc.c
 *
 * Description:
 *
 **************************************************************/

#include "fsFunc.h"
#include "fsLow.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

// Function to load bitmap to memory if signature matches
int loadSpace(int blockSize)
{

    int bitmapSize = (5 * blockSize); // Calculate the bitmap size based on the block size
    bitMapPtr = malloc(bitmapSize);   // Allocate memory for bitmap

    if (bitMapPtr == NULL)
    {
        printf("Failed to allocate memory for bitmap in freeSpace\n");
        return (-1);
    }

    // loading up the bitmap
    LBAread(bitMapPtr, 5, 1);

    return 1;
}

// Function to allocate required number of contiguous free blocks
unsigned int allocateFreeSpace(int numOfBlocks)
{
    // Hard coded starting block to 6 as first block 0=> VCB, Blocks 1 - 5 for bitmap
    unsigned int startingBlock = 6;
    unsigned int consecutiveBlocks = 0;       // Number of consecutive free blocks found
    unsigned int allocatedBlocks = 0;         // Starting block index of the allocated space
    unsigned int blockNumber = startingBlock; // Current block being checked

    while (blockNumber < vcb->totalBlocks)
    {
        if (consecutiveBlocks == numOfBlocks)
        {
            // If required no: of contiguous free blocks found, break the loop
            allocatedBlocks = blockNumber - numOfBlocks;
            break;
        }
        // Get the byte from the bitmap corresponding to the block
        unsigned char temp = bitMapPtr[blockNumber / 8];
        int bitPosition = blockNumber % 8; // Get the bit position within the byte

        // Check if the current block is free (bit is 0)
        // If we have a byte temp with a value of 0b10101010 and we want to check
        // the status of the 3rd bit (bit position 2).
        // The bitPosition variable would be set to 2, indicating the 3rd bit.
        // The expression temp >> (7 - bitPosition) would become temp >> (7 - 2),
        // which simplifies to temp >> 5.
        // Shifting the bits of temp 5 positions to the right, we get 0b00000101.
        //  The expression ((temp >> (7 - bitPosition)) & 1) would then be
        //(0b00000101 & 0b00000001), which equals 0b00000001.
        // The result is 1, indicating that the 3rd bit in temp is set to 1,
        // which means the corresponding block is used (not free).

        if (((temp >> (7 - bitPosition)) & 1) == 0)
        {
            if (consecutiveBlocks == 0)
            {
                // If it's the first free block encountered,
                // update the starting block index of the allocated space
                allocatedBlocks = blockNumber;
            }

            consecutiveBlocks++; // Increment count of consecutive free blocks
        }
        else
        {
            consecutiveBlocks = 0; // Reset the count if a used block is encountered
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
        unsigned int bitIndex = i % 8;  // Get the bit index within the byte
                                        // Set the corresponding bit to 1 to mark it as used
        bitMapPtr[byteIndex] |= (1 << (7 - bitIndex));
    }
    // Return the starting block index of the allocated space
    return allocatedBlocks;
}

// Function to find no: of blocks needed based on blob size
int blocksNeeded(int size)
{
    if (size == 0)
    {
        return 1;
    }

    int blocksNeeded = (size + (vcb->blockSize - 1)) / (vcb->blockSize);

    return blocksNeeded;
}

void createDir(char *name, DE *dirEntry, DE *parent)
{

    DE directory[totDirEnt];

    int blocksSpanned = blocksNeeded((totDirEnt * sizeof(DE)));
    int buffSize = blocksSpanned * sizeOfBlock;
    unsigned char *buffer = malloc(buffSize * sizeof(char));

    int dirSize = totDirEnt * sizeof(DE);
    int location = allocateFreeSpace(blocksSpanned);

    // set empty DE passed, that will live in parent dir.
    dirEntry->location = location;
    dirEntry->fileSize = dirSize;
    dirEntry->dirBlocks = blocksSpanned;
    strcpy(dirEntry->fileName, name); // make sure someone doesnt pass long name.
    strcpy(dirEntry->fileType, "d");
    time(&(dirEntry->created));
    time(&(dirEntry->lastModified));

    // pointer to self
    directory[0].location = location;
    printf("\n New directory location : %d\n", directory[0].location);
    directory[0].fileSize = (dirSize); // 2880
    directory[0].dirBlocks = blocksSpanned;
    strcpy(directory[0].fileName, ".");

    strcpy(directory[0].fileType, "d"); // 0x00; //0 denotes directory
    time(&(directory[0].created));
    time(&(directory[0].lastModified));

    // pointer to parent
    strcpy(directory[1].fileName, "..");
    directory[1].fileSize = parent->fileSize; // 2880
    strcpy(directory[1].fileType, "d");
    directory[1].dirBlocks = parent->dirBlocks;
    directory[1].created = parent->created;
    time(&(directory[1].lastModified));
    directory[1].location = parent->location;

    // initialize file name to see if empty
    for (int i = 2; i < totDirEnt; i++)
    {
        directory[i].fileName[0] = '\0';
        directory[i].dirBlocks = -1;
        directory[i].fileSize = 0;
        directory[i].location = -1;
        directory[i].fileType[0] = '\0';
    }

    // copy dir into buffer so fits into blocks of 512
    // copy buff to ParseDir
    unsigned char *buffLocation = buffer;
    for (int i = 0; i < totDirEnt; i++)
    {
        memcpy(buffLocation, &directory[i], sizeof(DE));
        buffLocation += sizeof(DE);
    }

    // write directory to volume
    int blocksWritten = LBAwrite(buffer, blocksSpanned, location);
    if (blocksWritten != blocksSpanned)
    {
        printf("error: writing to volume in mfs.c, createDir()\n");
    }

    // mark freespace used to create dir.
    // setTheBlocks(location, blocksSpanned);

    free(buffer);
    buffer = NULL;
    // printf("\nEND createDir()------------------------------------\n");
}

// if directories become dynamic will have to ask for new memory location and free old ones.
int writeDirToVolume(DE *dirToWrite)
{
    int blocksSpanned = dirToWrite[0].dirBlocks;

    int buffSize = dirToWrite[0].dirBlocks * sizeOfBlock;

    unsigned char *buffer = malloc(buffSize * sizeof(char));
    int dirSize = dirToWrite[0].fileSize;

    unsigned char *buffLocation = buffer;

    memcpy(buffer, dirToWrite, dirSize);

    // // write directory to volume
    int location = dirToWrite[0].location; // should be "." DE of directory pointing to self.

    int blocksWritten = LBAwrite(buffer, blocksSpanned, location);
    if (blocksWritten != blocksSpanned)
    {
        printf("error: writing to volume in mfs.c, createDir()\n");
    }

    free(buffer);
    buffer = NULL;

    return 0;
}

int reloadCurrentDir(DE *directory)
{
    // printf("\nreloadCurrentDir() ---------------------\n");

    int blocksSpanned = directory[0].dirBlocks;
    int buffSize = blocksSpanned * sizeOfBlock;
    char *buffer = malloc(buffSize * sizeof(char));

    uint64_t blocksRead = LBAread(buffer, blocksSpanned, directory[0].location);
    if (blocksRead != blocksSpanned)
    {
        printf("Error lbaREad fsInit.c, initFileSystem, load root to current dir\n");
        return -1;
    }

    unsigned char *buffLocation = buffer;
    for (int i = 0; i < totDirEnt; i++)
    {
        if (memcpy(dir[i], buffLocation, sizeof(DE)) == NULL)
        {
            printf("Error memcpy in fsInit.c, initFileSystem copy buffer to dir\n");
            return -1;
        }
        buffLocation += sizeof(DE);
    }
    return 0;
}

// get information on a given path from PPI struct.
PP *parsePath(const char *pathname)
{
    // printf("\nWELCOME TO parse path()\n");

    if (pathname == NULL || strlen(pathname) == 0)
    {
        printf("\nInvalid Path - empty or null\n");
        return NULL;
    }

    // reset previous values in struct.
    parseInfo.parentDirPtr = NULL;
    parseInfo.parentDirName = NULL;
    parseInfo.index = -1;
    parseInfo.exists = -2;
    strcpy(parseInfo.fileType, "");
    strcpy(parseInfo.name, "");
    parseInfo.path = NULL;

    // copy path so it wont be modified by strtok
    char path[strlen(pathname) + 1];
    strcpy(path, pathname); // error check.

    int absolutePath = pathname[0] == '/';

    // path not empty check first element if / then its starting at root
    //  Absolute path and current directory is not root.
    if (absolutePath && strcmp(pathname, currPath) == 0)
    {
        // load root. copy into parseDir.
        int blocksSpanned = blocksNeeded((totDirEnt * sizeof(DE)));
        int buffSize = blocksSpanned * BLOCK_SIZE;
        char buffer[buffSize];

        uint64_t blocksRead = LBAread(buffer, blocksSpanned, vcb->rootLocation);
        if (blocksRead != blocksSpanned)
        {
            printf("\nError during read in Parse path\n");
            return NULL;
        }

        // copy buff to ParseDir
        if (memcpy(parseDir, buffer, sizeof(parseDir)) == NULL)
        {
            printf("\nError memcpy in parsePath\n");
        }
    }
    else
    {
        // we must maintain a current directory. root is loaded as starting current directory.

        //   copy currentDir into parseDir to work with and change in case its
        //   a bad path we can scrap it without loding current dir.
        for (int i = 0; i < totDirEnt; i++)
        {
            parseDir[i].location = dir[i]->location;
            parseDir[i].fileSize = dir[i]->fileSize;
            parseDir[i].dirBlocks = dir[i]->dirBlocks;
            strcpy(parseDir[i].fileName, dir[i]->fileName);
            strcpy(parseDir[i].fileType, dir[i]->fileType);
            parseDir[i].lastModified = dir[i]->lastModified;
            parseDir[i].created = dir[i]->created;
        }
    }

    // tokenize the path
    char tokenizedPath[50][256]; // max 20 items in in a path.
    int numTokens = 0;
    char *token = strtok(path, "/");

    while (token != NULL)
    {
        strcpy(tokenizedPath[numTokens], token);
        numTokens++;
        token = strtok(NULL, "/");
    }

    // search path.
    int found = 0;       // 0 not found, 1 found;
    int foundIndex = -1; // j index in for loop, corresponds to inex in directory we are searching
    int endPathIndex = -1;
    parseInfo.exists = -1; // if last element is not found and set to 0 last element was not found.

    int i;
    for (i = 0; i < numTokens; i++)
    {
        int j;
        for (j = 0; j < totDirEnt && found == 0; j++)
        {
            // filenames match
            if (strcmp(parseDir[j].fileName, tokenizedPath[i]) == 0)
            {
                // make sure all filename tokens in path except for the last one are directories
                if (((numTokens - i) > 1) && (strcmp(parseDir[j].fileType, "d") == 0))
                {
                    found = 1;
                    foundIndex = j;
                }
                else if ((numTokens - i) == 1) // last token so we can mark if it exists or not.
                {
                    // printf("last token found : %s\n", parseDir[j].fileName);
                    parseInfo.exists = 0;
                    found = 1;
                    foundIndex = j;
                    endPathIndex = j;
                }
                else
                {
                    // token found but its a file when it should be a directory.
                    // except for last token in path.
                    found = 0;
                }
            }

        } // End for

        // path is valid except for last term.
        // the last token in the path name does not exist, so file or dir does not exist.
        // but still load 2nd to last directory.
        if ((numTokens - i) == 1 && found == 0)
        {
            found = 1; // not actally found, but still want to load 2nd to last directory.
        }

        // Went through directory and did not find a match for particular token.
        if (found == 0)
        {
            printf("\ninvalid path token : %s\n", tokenizedPath[i]);
            return NULL;
        }

        // token found get dir
        // load directory for searching except for last token in path.
        if ((numTokens - i) > 1)
        {
            if (foundIndex == -1)
            {
                return NULL;
            }

            // turn this into helper functio used twice.
            int blocksSpanned = blocksNeeded((totDirEnt * sizeof(DE)));
            int buffSize = blocksSpanned * BLOCK_SIZE;
            char buffer[buffSize];

            uint64_t blocksRead = LBAread(buffer, blocksSpanned, parseDir[foundIndex].location);
            if (blocksRead != blocksSpanned)
            {
                printf("\nError in parse path\n");
                return NULL;
            }

            // copy buff to ParseDir
            if (memcpy(parseDir, buffer, sizeof(parseDir)) == NULL)
            {
                printf("\nError memcpy in parsePath\n");
            }
        }

        // reset found for next search
        found = 0;
        foundIndex = -1;

    } // END FOR.

    // final path information that that will be returned.
    // when path passed is / (root);
    if (strcmp(pathname, "/") == 0)
    {
        endPathIndex = 0; // will access 0th index of root dir "." , pointer to itself.
    }

    // if endpathe is -1. last element did no exist, file type will be blank, if path is "/" strtok delim does not count it as a token.
    char *fileType = (endPathIndex > -1) ? parseDir[endPathIndex].fileType : "";

    strcpy(parseInfo.fileType, fileType);
    parseInfo.index = endPathIndex;
    parseInfo.parentDirPtr = parseDir;
    if (numTokens == 0)
    {
        // means only root was supplied "/".
        // delimeter is / so tokens will be 0.
        parseInfo.exists = 0;
    }

    // set name for last token, even if it doesnt exits.
    char lastElementName[256] = "";
    if (numTokens > 0)
    {
        printf("\ntoken[i - 1] = %s\n", tokenizedPath[numTokens - 1]);
        strcpy(lastElementName, tokenizedPath[numTokens - 1]);
    }
    else
    {
        // printf("last elemnt strcpy to root / \n");
        strcpy(lastElementName, "/");
    }

    // set name
    strcpy(parseInfo.name, lastElementName);

    int dotOrDotDot = -1; // if equals 0 . or .. is first element in path.
    // if relative path starting with .. replace .. with dir name.
    if (strcmp(pathname, "..") == 0)
    {
        dotOrDotDot = 0;

        // copy path so it wont be modified by strtok
        int lenOfCurrPath = strlen(currPath);
        char pathCopy[lenOfCurrPath + 1];
        strcpy(pathCopy, currPath); // error check.

        // tokenize the path
        char tokenCurrPath[50][256]; // max 20 items in in a path.
        int numCurrentTokens = 0;
        char *currToken = strtok(pathCopy, "/");

        while (currToken != NULL)
        {
            strcpy(tokenCurrPath[numCurrentTokens], currToken);
            numCurrentTokens++;
            currToken = strtok(NULL, "/");
        }

        // test display path.
        // printf("1. numtokens: %d\n", numCurrentTokens);
        printf("\ndisplay tokenized path\n");
        for (int i = 0; i < numCurrentTokens; i++)
        {
            printf("%s\n", tokenCurrPath[i]);
        }

        char *newPath;
        if (numCurrentTokens <= 1)
        {
            //.. is root "/"
            newPath = malloc(2 * sizeof(char));
            strcpy(newPath, "/");
            strcpy(parseInfo.name, "/");
            parseInfo.path = malloc(strlen(newPath) + 1);
            strcpy(parseInfo.path, newPath);
        }

        if (numCurrentTokens > 1)
        {
            strcpy(parseInfo.name, tokenCurrPath[numCurrentTokens - 2]);
            // printf("parse info name updated to : %s\n", parseInfo.name);

            // build new path.
            newPath = malloc(2 * sizeof(char));
            strcpy(newPath, "/");
            // printf("copy newpath / : %s\n", newPath);

            for (int i = 0; i < numCurrentTokens - 2; i++)
            {
                newPath = realloc(newPath, (strlen(newPath) + strlen(tokenCurrPath[i]) + 1));
                strcat(newPath, tokenCurrPath[i]);

                if (i != numCurrentTokens - 3)
                {
                    strcat(newPath, "/");
                }
            }
            // printf("copy newpath after for loop : %s\n", newPath);

            parseInfo.path = malloc(strlen(newPath) + 1);

            // update name
            strcpy(parseInfo.path, newPath);
        }
    }

    if (pathname[0] == '/')
    { // absolute path, if valid just use pathname.
        // copy pathname, dont modify original
        char pathnameCopy[strlen(pathname) + 1];
        strcpy(pathnameCopy, pathname);

        // create substring including / and last token to be ommited from path name.
        char substring[strlen(parseInfo.name) + 2];
        strcpy(substring, "/");
        strcat(substring, parseInfo.name);

        // removing matching substring to create path to parent dir.
        char *matchingSubString;
        int substringLen = strlen(substring) + 1;
        while (matchingSubString = strstr(pathnameCopy, substring))
        {
            *matchingSubString = '\0';
            strcat(pathnameCopy, matchingSubString);
        }

        // remove last token from string.

        parseInfo.path = malloc(strlen(pathnameCopy) + 1); // has a _+1 already for \0.
        strcpy(parseInfo.path, pathnameCopy);
    }
    else
    {
        // relative path, concat absolute path.
        if (numTokens > 1 && dotOrDotDot == -1)
        {
            parseInfo.path = malloc(strlen(pathname) + strlen(currPath) + 1);
            for (int i = 0; i < numTokens - 1; i++)
            {
                strcat(parseInfo.path, tokenizedPath[i]);
            }
        }
        else if (numTokens <= 1 && dotOrDotDot == -1)
        {
            parseInfo.path = malloc(strlen(pathname) + 1);
            strcpy(parseInfo.path, currPath);
        }
    }
    return &parseInfo;
} // END parsePath.

DE *findEmptyDE(DE *parentDir)
{

    for (int i = 0; i < totDirEnt; i++)
    {
        if (strcmp(parentDir[i].fileName, "") == 0)
        {
            return &parentDir[i];
        }
    }
    printf("NO empy DE in parent dir - return NULL, from findEmptyDE\n");
    return NULL;
}

void printCurrentDir(DE *dir[])
{
    for (int i = 0; i < totDirEnt; i++)
    {
        printf("\n");
        printf("Dfilename : %s\n", dir[i]->fileName);
        printf("Dlocation : %d\n", dir[i]->location);
        printf("Dfilesize: %lu\n", dir[i]->fileSize); // Use %lu for unsigned long
        printf("Dblockspanned : %d\n", dir[i]->dirBlocks);
        printf("DfileType : %s\n", dir[i]->fileType);
        printf("\n");
    }
}

void freeBlocks(int startBlock, int numberOfBlocks)
{
    // Calculate the byte and bit index for the start block
    int byteIndex = startBlock / 8;
    int bitIndex = startBlock % 8;

    int remainingBlocks = numberOfBlocks;

    while (remainingBlocks > 0)
    {
        // Check if the bit is currently set (block is used)
        int isSet = (bitMapPtr[byteIndex] >> (7 - bitIndex)) & 1;
        if (isSet)
        {
            // Clear the bit to mark the block as free
            bitMapPtr[byteIndex] &= ~(1 << (7 - bitIndex));
        }

        bitIndex++;
        remainingBlocks--;

        if (bitIndex >= 8)
        {
            // Move to the next byte in the bitmap
            byteIndex++;
            bitIndex = 0;
        }
    }
}


int createFile(char *filename, DE *dirEntry, DE *parent)
{

    // allocate blocks for file. 5120 default. so we dont waste space
    int blocksSpanned = blocksNeeded(BLOCK_SIZE * DEFAULT_BLOCKS_ALLOC_FOR_FILE);

    int fileLocation = allocateFreeSpace(DEFAULT_BLOCKS_ALLOC_FOR_FILE);
    printf("CREATE FILE: %s, loc: %d\n", filename, fileLocation);

    // modify dir entry no need to write file to disk, we have no data yet.
    // will have to write parent to disk for changes to take effect.
    dirEntry->location = fileLocation;
    dirEntry->fileSize = 0;
    dirEntry->dirBlocks = blocksSpanned;
    strcpy(dirEntry->fileName, filename);
    strcpy(dirEntry->fileType, "f");

    // mark blocks as used in spaceAllocation
    // setTheBlocks(fileLocation, blocksSpanned);

    return 0;
}