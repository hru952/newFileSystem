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

// I do not know why i need size  //I changed it.
char *fs_getcwd(char *pathname, size_t size)
{
    if (currPath != NULL)
    {
        strncpy(pathname, currPath, size);
        return pathname;
    }
    return NULL;
}

int fs_mkdir(const char *pathname, mode_t mode)
{

    PP *info = parsePath(pathname);

    if (info == NULL)
    {
        // either invalid path or dir by that name already exists.
        printf("\ninvalid path\n");
        return -1;
    }
    if (info->exists == 0)
    {
        printf("\ndir by that name already exists. \n");
        return -1;
    }

    // find empty DE in parent dir.
    // printf("\nFinding empty DE\n");
    DE *emptyDE = findEmptyDE(info->parentDirPtr);
    if (emptyDE == NULL)
    {
        printf("\ndirectory full\n");
        return -1;
    }
    // printf("\nFound empty DE\n");

    // create a directory
    createDir(info->name, emptyDE, info->parentDirPtr);
    printf("\n Created New Dir with given path \n");

    DE *updatedParentDir = malloc(totDirEnt * sizeof(DE));

    memcpy(updatedParentDir, info->parentDirPtr, (totDirEnt * sizeof(DE)));

    int retWriteDir = writeDirToVolume(updatedParentDir);
    if (retWriteDir < 0)
    {
        printf("error writing directory to volume, mfs.c, mkdir\n");
        return -1;
    }

    // if current dir contains the modified directory entry. reload it.
    if (strcmp(info->path, currPath) == 0)
    {

        // then we need to reload current directory to reflect changes.
        int retReload = reloadCurrentDir(updatedParentDir);

        free(info->path);
        info->path = NULL;
        return retReload;
    }

    free(info->path);
    info->path = NULL;

    return 0;
} // End fs_mkdir

// Function to know if the directory entry is file or dir
int CheckFileOrDir(const char *pathname)
{

    PP *parentDir = parsePath(pathname);

    if (parentDir == NULL)
    {
        return -2;
    }
    else
    {

        int result;
        if (strcmp(parentDir->parentDirPtr[parentDir->index].fileType, "f") == 0)
        {
            printf("This is a file\n");
            result = 0; // File
        }
        else if (strcmp(parentDir->parentDirPtr[parentDir->index].fileType, "d") == 0)
        {
            printf(" This is a directory\n");
            result = 1; // Directory
        }
        else
        {
            printf("Error! Not a file or directory\n");
            result = -1;
        }
        return result;
    }
}

// return 1 if file, 0 otherwise
int fs_isFile(char *filename)
{

    if (CheckFileOrDir(filename) == -2)
    {
        printf("From fs_isFile: Invalid pathname provided: Returning zero\n");
        return 0; // this means the pathname provided is invalid
    }
    else if (CheckFileOrDir(filename) == -1)
    {
        printf("From fs_isFile: Not a file or directory: Returning zero\n");
        return 0; // not a file or directory
    }
    else if (CheckFileOrDir(filename) == 0)
    {
        return 1; // is a file
    }
    else
    {
        printf("From fs_isFile: Is a directory and not a file: Returning zero\n");
        return 0; // is a directory
    }
}

// return 1 if directory, 0 otherwise
int fs_isDir(char *pathname)
{
    // check if the pathname is a valid path that leads to a valid directory
    // call helper routine
    if (CheckFileOrDir(pathname) == -2)
    {
        printf("From fs_isDir: Invalid pathname provided: Returning zero\n");
        return 0; // this means the pathname provided is invalid
    }
    else if (CheckFileOrDir(pathname) == -1)
    {
        printf("From fs_isDir: Not a file or directory: Returning zero\n");
        return 0; // not a file or directory
    }
    else if (CheckFileOrDir(pathname) == 0)
    {
        printf("From fs_isDir: Is a file and not a directory: Returning zero\n");
        return 0; // is a file
    }
    else
    {
        return 1; // is a directory
    }
}

char cwdPath[256];

int fs_setcwd(char *pathname)
{ // linux chdir
    // set current working directory

    PP *cwd = parsePath(pathname);

    if (cwd->exists == 0)
    {
        if (strcmp(cwd->fileType, "f") == 0)
        {
            printf("is a file\n");
            return (-1);
        }
        else if (strcmp(cwd->fileType, "d") == 0) // update current directory to pathname directory
        {

            int blocksSpanned = blocksNeeded((totDirEnt * sizeof(DE)));
            int buffSize = blocksSpanned * sizeOfBlock;
            char buf[buffSize];

            printf("lbaread location %d\n", cwd->parentDirPtr[parseInfo.index].location);
            printf("pathinfor index %d\n", cwd->index);
            // read dir from disk into buffer
            // confirm these are good values. not reading  rood again location is different.
            LBAread(buf, blocksSpanned, cwd->parentDirPtr[parseInfo.index].location);

            // copy contents of buffer into the current directory, to update current directory
            char *buffptr = buf;
            for (int i = 0; i < totDirEnt; i++)
            {
                memcpy(dir[i], buffptr, sizeof(DE));
                buffptr += sizeof(DE);
            }

            printf("\n print current dir in CD\n");
            printCurrentDir(dir);
            printf(" before concat parse path %s\n", cwd->path);

            if (strcmp(cwd->name, ".") == 0)
            {
                strcat(cwd->path, cwd->parentDirPtr[cwd->index].fileName);
            }
            else if (strcmp(cwd->name, "..") == 0)
            {
                // get parent dir name
                strcat(cwd->path, cwd->parentDirPtr[cwd->index].fileName);
            }

            if ((strlen(cwd->path) == 1) && (strcmp(cwd->name, "/") != 0))
            {
                // parent dir is root. just concat name without adding "/"
                strcat(cwd->path, cwd->name);
            }
            else if (strcmp(cwd->name, "/") != 0)
            {
                // root is its own parent, so dont add / and name to build path, add "/"

                printf("setcwd add / to path\n");
                printf("name = %s \n", cwd->name);
                strcat(cwd->path, "/");
                strcat(cwd->path, cwd->name);
            }
            printf("after concat cwd->path %s\n", cwd->path);
            currPath = malloc(strlen(cwdPath) + 1);
            strcpy(currPath, cwd->path);
            printf("crruent path after copy %s\n", currPath);
        }
    }
    else if (cwd->exists == -1)
    {
        printf("Error : Directory does not exists\n");
        return (-1);
    }

    return 0;
}

// fs_opendir and fs_closedir below

DE openDirDe[totDirEnt]; // array used in fs_opendir

fdDir *fs_opendir(const char *pathname)
{
    // check if the given path exists or not
    int fileOrDirVal = CheckFileOrDir(pathname);

    if (fileOrDirVal == -2)
    {
        printf("Pathname is invalid\n");
        return NULL;
    }
    else if (fileOrDirVal == -1)
    {
        printf("Path is neither a file nor a directory\n");
        return NULL;
    }
    else if (fileOrDirVal == 0)
    {
        printf("Path is a not a directory\n");
        return NULL;
    }
    else if (fileOrDirVal == 1)
    {
        // Opening logic starts
        PP *parentDir = parsePath(pathname);
        printf("Directory opened: %s\n", parentDir->name);

        fdDir *openedDir = malloc(sizeof(fdDir));
        openedDir->dirEntryPosition = 0;
        // calculates the number of blocks needed to store the directory entries based on the size of
        // each directory entry and the total number of directory entries
        int numBlocks = blocksNeeded((totDirEnt * sizeof(DE)));
        int bufferSize = numBlocks * BLOCK_SIZE;
        char buffer[bufferSize];

        uint64_t numBlocksRead = LBAread(buffer, numBlocks, parentDir->parentDirPtr[parentDir->index].location);
        if (numBlocksRead != numBlocks)
        {
            return NULL;
        }

        // copy the contents of the buffer into the openDirDe array.
        if (memcpy(openDirDe, buffer, sizeof(openDirDe)) == NULL)
        {
            printf("Error in the memcpy operation\n");
        }
        openedDir->directoryStartLocation = openDirDe[0].location;
        openedDir->openDirPtr = openDirDe;

        openedDir->d_reclen = sizeof(fdDir);
        printf("Size of fdDir: %ld\n", sizeof(fdDir));
        return openedDir;
    }
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    int index = dirp->dirEntryPosition;
    int validEntry = 0;
    int exit = 0;
    struct fs_diriteminfo *dirItem;
    dirItem = malloc(sizeof(struct fs_diriteminfo));

    while (validEntry == 0 && exit == 0)
    {
        if (index == (totDirEnt))
        {
            exit = -1;
            return NULL;
        }

        if ((strcmp(dirp->openDirPtr[index].fileName, "")) != 0)
        {
            strncpy(dirItem->d_name, dirp->openDirPtr[dirp->dirEntryPosition].fileName, 255);

            dirItem->d_reclen = dirp->openDirPtr[dirp->dirEntryPosition].fileSize;

            validEntry = 1;
            exit = -1;
        }
        index++;
    }

    dirp->dirEntryPosition = index;
    return dirItem;
}

int fs_closedir(fdDir *dirp)
{
    if (dirp == NULL)
    {
        return (-1);
    }

    free(dirp);
    dirp = NULL;

    return 0;
}

int fs_rmdir(const char *pathname)
{
    PP *parseInfo = parsePath(pathname);

    if (parseInfo == NULL)
        return -1; // Directory not found or some unexpected error

    if (strcmp(pathname, "/") == 0)
    {
        return -1; // Should not delete root
    }

    if (strcmp(parseInfo->parentDirPtr[parseInfo->index].fileType, "d") != 0)
        return -1; // If it is not a directory

    fdDir *dirp = fs_opendir(pathname);

    struct fs_diriteminfo *di = fs_readdir(dirp);

    while (di != NULL)
    {

        if (di->d_name[0] == '.' || strcmp(di->d_name, "..") == 0 || strcmp(di->d_name, "") == 0) // Check the '.', '..' and empty entries in a directory
            di = fs_readdir(dirp);
        else
        {
            return -1; // directory not empty
        }
    }

    freeBlocks(parseInfo->parentDirPtr[parseInfo->index].location, blocksNeeded(parseInfo->parentDirPtr[parseInfo->index].fileSize));

    strcpy(parseInfo->parentDirPtr[parseInfo->index].fileName, "");
    parseInfo->parentDirPtr[parseInfo->index].fileSize = 0;
    parseInfo->parentDirPtr[parseInfo->index].location = -1;
    parseInfo->parentDirPtr[parseInfo->index].dirBlocks = 0;

    // write the directory changes to disk
    writeDirToVolume(parseInfo->parentDirPtr);

    if (strcpy(parseInfo->path, currPath) == 0)
    {

        reloadCurrentDir(parseInfo->parentDirPtr);
    }

    return 0;
}

int fs_delete(char *filename)
{
    PP *parseInfo = parsePath(filename);

    if (parseInfo == NULL)
        return -1; // file not found

    if (strcmp(parseInfo->parentDirPtr[parseInfo->index].fileType, "f") != 0)
        return -1; // If not a file

    strcpy(parseInfo->parentDirPtr[parseInfo->index].fileName, "");
    parseInfo->parentDirPtr[parseInfo->index].location = -1;
    parseInfo->parentDirPtr[parseInfo->index].fileSize = 0;
    parseInfo->parentDirPtr[parseInfo->index].dirBlocks = 0;

    // Use freeTheBlocks(location, blocksNeeded(size of file))
    freeBlocks(parseInfo->parentDirPtr[parseInfo->index].location, blocksNeeded(parseInfo->parentDirPtr[parseInfo->index].fileSize));

     // write the directory changes to disk
    writeDirToVolume(parseInfo->parentDirPtr);

    if (strcpy(parseInfo->path, currPath) == 0)
    {

        reloadCurrentDir(parseInfo->parentDirPtr);
    }

    return 0;
}

int fs_stat(const char *path, struct fs_stat *buf)
{
    PP *pathInfo = parsePath(path);

    if (pathInfo == NULL)
        return -1; // Does not exist or error finding the desired file or directory

    buf->st_size = pathInfo->parentDirPtr[pathInfo->index].fileSize;
    buf->st_blksize = sizeOfBlock;
    buf->st_blocks = pathInfo->parentDirPtr[pathInfo->index].dirBlocks;
    buf->st_createtime = pathInfo->parentDirPtr[pathInfo->index].created;
    buf->st_modtime = pathInfo->parentDirPtr[pathInfo->index].lastModified;

    return 0;
}