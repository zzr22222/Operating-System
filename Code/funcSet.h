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
Block* disk; // ���̿ռ�
static void* disk_space = (void*)0; 
int shmid; // �����ڴ��ID��
char* fat; // ָ��FAT��
//��·���йص����ݽṹ
Fcb* open_path[16]; // ��������FCB��
char* open_name[16]; //���������ļ���
short current; // ��¼��ǰĿ¼�����
static const int FCB_LIST_LEN = sizeof(Block) / sizeof(Fcb); //һ������ʹ�õ�FCB����
//�ź���
sem_t* sem_read, * sem_write;
//�������ʼ���ĺ���
Block* getDisk();
void freeDisk();
void createDisk();
void createBootBlock();
void createDataBlock();
void createFat();
void createDir(Fcb* fcb, short block_number, short parent_number);
/* ���ܺ�������ʵ�������������������ҪЧ��*/
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
//�ļ������ĺ���
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
//һЩ��������ĺ���
void printPathInfo();
char* getArg(char* str);
char* getCmd(char* cmd);
int Shell();
int split(char** arr, char* str, const char* delims);