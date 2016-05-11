// FileService.h
// simple file service

#ifndef _FILESERVICE_H_
#define _FILESERVICE_H_

#define GOOD          1
#define BAD          -1
#define EMPTY        -1
#define UNUSED     -100
#define UNFOUND    -101
#define UNKNOWN    -102
#define NO_FD      -103  // all file descriptors in use
#define FM_EOF     -104  // no more content
#define ILL_PARAM  -105  // some parameter was invalid
#define BUF_SMALL  -106  // your buffer too small
#define CHK_OBJ      80  // existing obj
#define OPEN_OBJ     81  // existing obj
#define READ_OBJ     82
#define WRITE_OBJ    83
#define CLOSE_OBJ    84
#define OPEN_NEW_OBJ 85 // create new file

#define MAX_FD 20                             // # of avail FD
#define VALID_FD_RANGE(fd) (-1 < fd && fd < MAX_FD) // FD #-range check

#define A_MT  0xF000 // Attribute Identifier Flag Mask Table for mask & compare
#define A_DIR 0x4000 // DIRectory type
#define A_REG 0x8000 // REGular file type

#define A_RWXU 00700 // owner: read, write, execute
#define A_RWXG 00070 // group: read, write, execute
#define A_RWXO 00007 // other: read, write, execute
#define A_ROTH 00004 // other: read
#define A_WOTH 00002 // other: write
#define A_XOTH 00001 // other: execute

// macros to extract informaton from "mode" field of "attr_t"
// is it a directory or regular file?
#define A_ISDIR(mode) (((mode) & A_MT) == A_DIR)
#define A_ISREG(mode) (((mode) & A_MT) == A_REG)

#define MODE_EXEC (0777 | A_REG)
#define MODE_FILE (0666 | A_REG)
#define MODE_READFILE (0444 | A_REG)
#define MODE_DIR (0777 | A_DIR)

#define END_DIR_INODE ((unsigned int)(~0)) // end marker of directory content

typedef struct {
   int inode,   // inode on the device
       mode,    // file access mode
       dev,     // PID of file system
       nlink,   // # of links to the file
       size;    // file size
   char *data;  // file content
} attr_t;

typedef struct { // directory type
   int inode,
       mode,
       size;
   char *name,
        *data;
} dir_t;

typedef struct {      // file descriptor type
   int owner,         // PID, -1 means not used
       offset;        // can be negative
   dir_t *item,
         *owning_dir; // dir where `item' resides
} fd_t;

void FileService();                // File Server & its routines
int ChkObj(char *, attr_t *);
int OpenObj(char *, int, int *);
int CloseObj(int, int);
int ReadObj(int, char *, int, int *);
int CanAccessFD(int, int);
int AllocFD(int);
void FreeFD(int);
dir_t *FindName(char *);
dir_t *FindNameSub(char *, dir_t *);
void Dir2Attr(dir_t *, attr_t *);

#endif

