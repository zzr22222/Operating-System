**Project Name:** Simple File System Design
**Project Goal:** 

1. Create a 100MB file or a 100MB shared memory.
2. Try to design a small C program on your own, using the 100MB space allocated in step 1 (via mmap or shared memory). Then, assume this 100MB space as a blank disk and design a simple file system to manage this blank disk. Provide the basic data structure for file and directory management, draw a diagram of the basic structure of the file system, and describe the basic operation interfaces.
3. Based on step 1, implement some file operation interfaces: create directory (mkdir), delete directory (rmdir), rename, create file (open), modify file, delete file (rm), and view the file system directory structure (ls). Note: All implementations should be done in memory.
4. Refer to the chapter on process synchronization and use the semaphore mechanism to implement mutually exclusive access to the above file system from multiple terminals. In the system, a file can be read by multiple processes but not written to; or, it can be written to by only one operation, without allowing read access.

**Solution:**

- funcSet.c: Writing of the shell interface, creation and initialization of disk shared memory, and functional modules including: operations for `mkdir`, `rmdir`, `rename`, `open`, `write`, `rm`, `ls`, `cd`, `help` commands, and other auxiliary command functions such as `printPathInfo`, `getArg`, `getCmd`.
- funcSet.h: Various functions for managing a simulated disk and file operations.
- main.c: Importing interface functions and utility functions.
- valSet.h: Defines structures such as `Block`, `Disk`, `BootBlock`, `Fat`, `Fcb`, etc.