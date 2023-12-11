#pragma once
#include <fcntl.h>
#include <sys/stat.h>
#define USED -1
#define FREE 0
#define FAT_BLOCK 1
#define DATA_BLOCK 8
static const unsigned int BLOCK_NUM = 25600; // 25MB
static const unsigned int DATA_NUM = 25600 - DATA_BLOCK;
static const unsigned int BLOCK_SIZE = 4096;// 4MB
static const unsigned int DISK_SIZE = 104857600; // 100MB
typedef struct {
    char space[4096];
} Block;
typedef struct {
    Block data[25600];// 25600 * Block(4096) = 104857600(100MB)
} Disk;
typedef struct {
    char disk_name[32];
    short disk_size;
    Block* fat_block;   // 指向FAT表的起始位置
    Block* data_block;  // 指向数据块的起始位置
} BootBlock;
typedef struct {
    char id;
} Fat;
typedef struct
{
    unsigned short year;
    unsigned short month;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
} Datetime;
typedef struct
{
    char name[11]; // 文件名
    char ext[3]; // 拓展名
    Datetime datetime; // 创建时间
    short start_block; // 第一个起始块
    unsigned short size; // 块长度
    char is_dir; // 是否是目录
    char is_used; // 指示目录项是否使用了
} Fcb;