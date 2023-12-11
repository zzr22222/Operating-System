#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "valSet.h"
#define READ_MAX 256
/*disk*/
Block* disk; // 磁盘空间
static void* disk_space = (void*)0; 
int shmid; // 共享内存的ID号
char* fat; // 指向FAT表
//与路径有关的数据结构
Fcb* open_path[16]; // 存放最近的FCB快
char* open_name[16]; //存放最近的文件名
short current; // 记录当前目录的深度
static const int FCB_LIST_LEN = sizeof(Block) / sizeof(Fcb); //一个块能使用的FCB数量
//信号量
sem_t* sem_read, * sem_write;
//创建与初始化的函数
Block* getDisk();
void freeDisk();
void createDisk();
void createBootBlock();
void createDataBlock();
void createFat();
void createDir(Fcb* fcb, short block_number, short parent_number);
/* 功能函数，在实现下面的其他操作有重要效果*/
Block* getBlock(int block_number);
void setCurrentTime(Datetime* datetime);
int getFreeBlock(int size);
int getBlockNum(int size);
Fcb* searchFcb(char* path, Fcb* root);
Fcb* getFreeFcb(Fcb* fcb);
Fcb* createFcb(Fcb* fcb, char* name, char is_dir, int size);
Fcb* getParent(char* path);
char* getPathLastName(char* path);
char* getAbsPath(char* path, char* abs_path);
//文件操作的函数
int Mkdir(char* path);
int Rmdir(char* path, Fcb* root);
int Rename(char* src, char* dst);
int Open(char* path);
int Write(char* path);
int Rm(char* path, Fcb* root);
void Ls();
void Lls();
int Cd(char* path);
int Help();
//一些处理命令的函数
void printPathInfo();
char* getArg(char* str);
char* getCmd(char* cmd);
int Shell();
int split(char** arr, char* str, const char* delims);