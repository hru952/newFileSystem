/**************************************************************
 * Class:  CSC-415-02 Summer 2023
 * Names: Saripalli Hruthika, Nixxy Dewalt, Alekya Bairaboina, Banting Lin
 * Student IDs: 923066687, 922018328, 923041428, 922404012
 * GitHub Name:
 * Group Name: Zombies
 * Project: Basic File System
 *
 * File: fsFunc.h
 *
 * Description: File system helper functions to interact with the shell
 *
 *
 **************************************************************/
#ifndef _FSFUNC_H
#define _FSFUNC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define totDirEnt 30
// #define blocksInVolume = ;
#define BLOCK_SIZE 512
#define sizeOfBlock 512
#define DEFAULT_BLOCKS_ALLOC_FOR_FILE 5

typedef struct VCB
{
    long signature;              // Unique number to validate the VCB
    int totalBlocks;             // To store total number of blocks
    int blockSize;               // To store size of a block
    unsigned int rootLocation;   // Location of root node
    unsigned int bitMapLocation; // Starting location of the freespace bitmap
} VCB;

typedef struct dirEntry
{
    char fileName[100];     // Name of file
    unsigned int location;  // Starting location of the directory blocks
    unsigned long fileSize; // Size of the blob
    char fileType[1];       // Flag to identify directory or normal blob
    int dirBlocks;          // Number of blocks it occupies
    time_t created;         // Created time
    time_t lastModified;    // Last modified time
} DE;

typedef struct parsePathInfo // for parsePath
{
    DE *parentDirPtr;
    char *parentDirName;
    int index;        // index in directory;
    int exists;       // 0 exists, -1 not exists, for last file or dir in the path
    char fileType[2]; // "f" = file, "d"= dir, is a c string.
    char name[256];   // last file or directory name.
    char *path;       // parent path. must free this when your done.

} PP;

unsigned char *bitMapPtr; // to hold the memory address of the free space bitmap.
VCB *vcb;                 // to hold address of the vcb
DE *dir[totDirEnt];

// Variables to store the path details of current directory
int currPathLen;
char *currPath; // free in close.

int loadSpace(int blockSize);
unsigned int allocateFreeSpace(int numOfBlocks);
// int totalNumOfBlocks(int size);
// void createNewDir(char * name, DE * dirEntry, DE * parent);
// int writeDir (DE * directory);
// int loadDir(DE * directory);
// DE * allocateDE (DE * parentDir);
int totalNumOfBlocks(int size);
void createNewDir(char *name, DE *dirEntry, DE *parent);
int dirToDisk(DE *directory);
int loadUpdatedDir(DE *directory);
DE *lookForFreeDE(DE *parentDir);

PP parseInfo;           // for parsePath
DE parseDir[totDirEnt]; // for parsePath
PP *parsePath(const char *pathname);

void freeBlocks(int startBlock, int numberOfBlocks);

int createNewFile(char *filename, DE *dirEntry, DE *parent);

#endif
