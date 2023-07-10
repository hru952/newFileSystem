//extends
struct Extent {
    unsigned int loc; // Starting location of the extent
    unsigned int count; // Total used blocks in the extent

}

//Directory Entry
struct DirectoryEntry{
    unsigned long size; // Size of the directory entry
    struct Extent location[50]; // Array of extents for storing data blocks
    char name[256]; // Name of the file
    time_t created; // Date time of creation
    time_t lastModified; // Date time of last modification
    time_t lastAccessed; // Date time of last access
    char fileExtension[16]; // File extension like .txt, pdf. 

}