/**************************************************************
* Class:  CSC-415-02 Summer 2023
* Names: Saripalli Hruthika, Nixxy Dewalt, Alekya Bairaboina, Banting Lin 
* Student IDs: 923066687, 922018328, 923041428, 922404012
* GitHub Name: hru952
* Group Name: Zombies
* Project: Basic File System
*
* File: mfs.c
*
* Description: 
*
**************************************************************/

#include <stdio.h>
#include <string.h>
#include "mfs.h"
#include "fsLow.h"
#include "fsFunc.h"

int fs_mkdir(const char *pathname, mode_t mode)
{
    printf("\nWELCOME TO fs_mkdir()\n");
    PP *info = parsePath(pathname);
    
    if (info == NULL)
    {
        // either invalid path or dir by that name already exists.
        printf("invalid path. fs_mkdir, mrs.c\n");
        return -1;
    }
    if (info->exists == 0)
    {
        printf("dir by that name already exists. fs_mkdir, mrs.c\n");
        return -1;
    }
    
    //find empty DE in parent dir.
    printf("\nFinding empty DE\n");
    DE *emptyDE = findEmptyDE(info->parentDirPtr);
    if (emptyDE == NULL)
    {
        printf("directory full, fs_mkdir\n");
        return -1;
    }
    printf("\nFound empty DE\n");

    //create a directory
    createDir(info->name, emptyDE, info->parentDirPtr);
    printf("\n Created New Dir\n");

    DE * updatedParentDir = malloc(totDirEnt * sizeof(DE));
    
    memcpy(updatedParentDir, info->parentDirPtr, (totDirEnt * sizeof(DE)));
 
    int retWriteDir = writeDirToVolume(updatedParentDir);
    if (retWriteDir < 0)
    {
        printf("error writing directory to volume, mfs.c, mkdir\n");
        return -1;
    }

    //if current dir contains the modified directory entry. reload it.
    if (strcmp(info->path, currPath) == 0)
    {

        //then we need to reload current directory to reflect changes.
    int retReload = reloadCurrentDir(updatedParentDir);

    free(info->path);
    info->path = NULL;
        return retReload;
    }


    free(info->path);
    info->path = NULL;

    return 0;
} //End fs_mkdir



