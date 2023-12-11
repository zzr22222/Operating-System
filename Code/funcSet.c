#include "funcSet.h"
/*------------------------------------Shell--------------------------------------------*/
int Shell()
{
    char buffer[64];
    char tmp_buffer[64];
    while (1) {
        printPathInfo();
        getCmd(buffer);
        if (strcmp(buffer, "mkdir") == 0) {
            Mkdir(getArg(buffer));
        }
        else if (strcmp(buffer, "rmdir") == 0) {
            Rmdir(getArg(buffer), open_path[current]);
        }
        else if (strcmp(buffer, "rename") == 0) {
            getArg(buffer);
            getArg(tmp_buffer);
            Rename(buffer, tmp_buffer);
        }
        else if (strcmp(buffer, "open") == 0) {
            Open(getArg(buffer));
        }
        else if (strcmp(buffer, "write") == 0) {
            Write(getArg(buffer));
        }
        else if (strcmp(buffer, "rm") == 0) {
            Rm(getArg(buffer), open_path[current]);
        }
        else if (strcmp(buffer, "ls") == 0) {
            Ls();
        }
        else if (strcmp(buffer, "lls") == 0) {
            Lls();
        }
        else if (strcmp(buffer, "cd") == 0) {
            Cd(getArg(buffer));
        }
        else if (strcmp(buffer, "exit") == 0) {
            return 0;
        }
        else if (strcmp(buffer, "help") == 0) {
            Help();
        }
        else if (strlen(buffer) != 0) {
            printf("[Shell] Unsupported command\n");
        }
        fflush(stdin);
    }
    return -1;
}
/*------------------------------------Shell--------------------------------------------*/

/*----------------------------------�������ʼ��----------------------------------------*/
//һЩ��ʼ���Ĳ���

// ���̳�ʼ��������100MB�ռ䣬Ȼ����ڵ�disk��
Block* getDisk()
{
    // ��ȡ�����ڴ��ʶ��
    shmid = shmget((key_t)1137, (size_t)sizeof(Disk), 0666 | IPC_CREAT);
    //�жϹ����ڴ��Ƿ����뵽
    if (shmid == -1) {
        fprintf(stderr, "[getDisk] Shmget failed\n");
        exit(EXIT_FAILURE);
    }
    //attach�����ڴ�Σ�������һ�������ڴ�ε�ָ��
    disk_space = shmat(shmid, (void*)0, 0);
    //�жϹ��ع����ڴ��Ƿ�ɹ�
    if (disk_space == (void*)-1) {
        fprintf(stderr, "[getDisk] Shmat failed\n");
        exit(EXIT_FAILURE);
    }
    //�������ڴ���Ϊ���̣�����
    disk = (Block*)disk_space;
    return disk;
}
//���ڽ�����ʱ���ͷŴ���
void freeDisk()
{
    if (shmdt(disk_space) == -1) {//�Ͽ������ڴ�ʧ��
        fprintf(stderr, "[releaseDisk] Shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, 0) == -1) {//ɾ�������ڴ�ʧ��
        fprintf(stderr, "[releaseDisk] Shmctl failed\n");
        exit(EXIT_FAILURE);
    }
}

//�ܵĳ�ʼ���Ĳ�������ʼ�������飬FAT�����ݿ�
void createDisk()
{
    createBootBlock();
    createFat();
    createDataBlock();
}
//��ʼ����������
void createBootBlock()
{
    BootBlock* boot_block = (BootBlock*)disk;
    strcpy(boot_block->disk_name, "zzr's Disk");
    boot_block->disk_size = sizeof(Block) * BLOCK_NUM;// ���̴�СΪ100MB
    boot_block->fat_block = disk + FAT_BLOCK;// ����ָ�뵽FAT��
    boot_block->data_block = disk + DATA_BLOCK;// ����ָ�뵽���ݿ�
}

// ��ʼ��FAT��
void createFat()
{
    fat = (char*)(disk + FAT_BLOCK);
    //�����������FAT��Ϊ�Ѿ���ʹ��
    for (int i = 0; i < FAT_BLOCK; i++) {
        fat[i] = USED;
    }
    //���������Ŀ�Ϊδʹ��
    for (int i = FAT_BLOCK; i < BLOCK_NUM; i++) {
        fat[i] = FREE;
    }
}

// ��ʼ�����ݿ�
void createDataBlock()
{
    //������Ҫ���ȴ���һ����Ŀ¼
    Fcb* root = (Fcb*)getBlock(DATA_BLOCK);
    createDir(root, DATA_BLOCK, DATA_BLOCK);
    current = 0;//��ʼ��Ŀ¼ָ���ָ��
    open_path[current] = root;
    open_name[current] = "Root";
}

// ��ʼ��Ŀ¼
void createDir(Fcb* fcb, short start_block, short parent_number)
{
    //ռ�ô�ʱ�����ݿ�,�����ר��������Ÿ��ļ���һ����Ŀ¼��
    fat[start_block] = USED;
    //��ȡ��ǰ��ʱ����Ϣ
    setCurrentTime(&fcb->datetime);
    //����FCB�����Ϣ
    fcb->start_block = start_block;
    fcb->size = 2 * sizeof(Fcb);
    fcb->is_used = 1;
    //���Ŀ¼ '.'������ǰĿ¼
    strcpy(fcb->name, ".");
    fcb->is_dir = 1;
    //������һ��Ŀ¼ '..',���ϼ�Ŀ¼
    Fcb* p = fcb + 1;
    memcpy(p, fcb, sizeof(Fcb));
    strcpy(p->name, "..");
    p->start_block = parent_number;
    p->size = -1;

    //������Ŀ¼���г�ʼ��
    for (int i = 2; i < FCB_LIST_LEN; i++) {
        p++;
        strcpy(p->name, "");
        p->is_used = 0;
    }
}

// ��ʼ��FCB
Fcb* createFcb(Fcb* fcb, char* name, char is_dir, int size)
{
    strcpy(fcb->name, name);
    fcb->is_dir = is_dir;
    setCurrentTime(&fcb->datetime);
    //���ҿ��п�
    int block_num = getFreeBlock(getBlockNum(size));
    if (block_num == -1) {
        printf("[initFcb] Disk has Fulled\n");
        exit(EXIT_FAILURE);
    }
    fcb->start_block = block_num;
    fcb->size = 0;
    fcb->is_used = 1;
    return fcb;
}
/*----------------------------------�������ʼ��----------------------------------------*/

/*-------------------------------------����ģ��-----------------------------------------*/
//��ʱ������Ϊ��ǰʱ��
void setCurrentTime(Datetime* datetime)
{
    time_t rawtime;
    time(&rawtime);
    struct tm* time = localtime(&rawtime);
    datetime->year = (time->tm_year + 1900);
    datetime->month = (time->tm_mon + 1);
    datetime->day = time->tm_mday;
    datetime->hour = time->tm_hour;
    datetime->minute = time->tm_min;
    datetime->second = time->tm_sec;
}

// ��ȡ��ǰ�Ŀ�ָ��
Block* getBlock(int start_block)
{
    return disk + start_block;
}

// ������������С��ȡ��Ҫռ�ö��ٿ�
int getBlockNum(int size)
{
    return (size - 1) / sizeof(Block) + 1;
}
//��ȡ���п�
int getFreeBlock(int size)
{
    int count = 0;
    for (int i = DATA_BLOCK; i < DATA_NUM; i++) {
        if (fat[i] == FREE) {
            count++;
        }
        else {
            count = 0;
        }
        if (count == size) {
            for (int j = 0; j < size; j++) {
                fat[i - j] = USED;
            }
            return i-size+1;
        }
    }
    return -1;
}

// ����·���͵�ǰĿ¼��FCB��Ѱ��λ��

//��������·������FCB��
Fcb* searchFcb(char* path, Fcb* root)
{
    char _path[64];
    strcpy(_path, path);
    char* name = strtok(_path, "/");
    char* next = strtok(NULL, "/");
    Fcb* p = root;
    for (int i = 0; i < FCB_LIST_LEN; i++) {
        if (p->is_used == 1 && strcmp(p->name, name) == 0) {
            if (next == NULL) {
                return p;
            }
            return searchFcb(path + strlen(name) + 1, (Fcb*)getBlock(p->start_block));
        }
        p++;
    }
    return NULL;
}

// ��ȡ��ǰ�ľ���·��
char* getAbsPath(char* path, char* abs_path)
{
    char abs_path_arr[16][16];
    int len;
    for (len = 0; len <= current; len++)
        strcpy(abs_path_arr[len], open_name[len]);
    char _path[64];
    strcpy(_path, path);
    char* name = strtok(_path, "/");
    char* next = name;
    while (next != NULL) {
        name = next;
        next = strtok(NULL, "/");
        if (strcmp(name, ".") == 0)
            continue;
        else if (strcmp(name, "..") == 0)
            len--;
        else
            strcpy(abs_path_arr[len++], name);
    }
    char* p = abs_path;
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < strlen(abs_path_arr[i]); j++)
            *p++ = abs_path_arr[i][j];
        *p++ = '-';
    }
    *(p - 1) = 0;
    return abs_path;
}

// ��ȡ���е�FCB��
Fcb* getFreeFcb(Fcb* fcb)
{
    for (int i = 0; i < FCB_LIST_LEN; i++) {
        if (fcb->is_used == 0) {
            return fcb;
        }
        fcb++;
    }
    return NULL;
}



// ��ȡ��Ŀ¼
Fcb* getParent(char* path)
{
    // get the parent path string in parent_path
    char parent_path[64];
    strcpy(parent_path, path);
    for (int i = strlen(parent_path); i >= 0; i--) {
        if (parent_path[i] == '/') {
            parent_path[i] = 0;
            break;
        }
        parent_path[i] = 0;
    }
    // ��ʼѰ�Ҹ��ڵ��FCB
    Fcb* parent;
    if (strlen(parent_path) != 0) {
        Fcb* parent_pcb = searchFcb(parent_path, open_path[current]);
        if (parent_pcb == NULL) {
            return NULL;
        }
        parent = (Fcb*)getBlock(parent_pcb->start_block);
    }
    else {
        parent = open_path[current];
    }
    return parent;
}

// return the last name of a path
char* getPathLastName(char* path)
{
    char _path[64];
    strcpy(_path, path);
    char* name = strtok(_path, "/");
    char* next = name;
    while (next != NULL) {
        name = next;
        next = strtok(NULL, "/");
    }
    return name;
}
/*-------------------------------------����ģ��-----------------------------------------*/

/*------------------------------------�ļ���������---------------------------------------*/

// mkdir 
int Mkdir(char* path)
{
    // Ѱ�Ҹ�·����FCB�ļ�
    Fcb* res = searchFcb(path, open_path[current]);
    if (res) {
        printf("[Mkdir] %s is existed\n", path);
        return -1;
    }
    // Ѱ�Ҹ��ڵ�
    Fcb* parent = getParent(path);
    if (parent == NULL) {
        printf("[Mkdir] Not found %s\n", path);
        return -1;
    }
    // �����µ�FCB�ڵ�
    Fcb* fcb = getFreeFcb(parent);
    char* name = getPathLastName(path);
    createFcb(fcb, name, 1, sizeof(Block));
    fcb->size = sizeof(Fcb) * 2;
    parent->size += sizeof(Fcb);

    // ��ʼ��FCB����Ϣ
    Fcb* new_dir = (Fcb*)getBlock(fcb->start_block);
    createDir(new_dir, fcb->start_block, parent->start_block);
    return 0;
}

// rmdir 
int Rmdir(char* path, Fcb* root)
{
    Fcb* fcb = searchFcb(path, root);
    if (fcb && fcb->is_dir == 1) {
        if (strcmp(fcb->name, ".") == 0 || strcmp(fcb->name, "..") == 0) {
            printf("[Rmdir] You can't delete %s\n", fcb->name);
            return -1;
        }
        // ��ȡҪ�Ƴ��Ŀ�
        Fcb *p = (Fcb *) getBlock(fcb->start_block) + 2;// ǰ�������ǵ�ǰĿ¼�͸�Ŀ¼
        for (int i = 2; i < FCB_LIST_LEN; i++) {
            if (p->is_used == 0) continue;
            else if (p->is_dir) Rmdir(p->name, p);
            else Rm(p->name, p);
            p++;
        }
        // �ͷŵ�FAT���ϵı��
        for (int i = 0; i < getBlockNum(fcb->size); i++)
            fat[fcb->start_block + i] = FREE;
        fcb->is_used = 0;
        // �ı���ڵ��size
        root->size -= sizeof(Fcb);
    }
    else {
        printf("[Rmdir] Not found %s\n", path);
        return -1;
    }
    return 0;
}

// rename
int Rename(char* src, char* dst)
{
    Fcb* fcb = searchFcb(src, open_path[current]);
    if (fcb) {
        if (strcmp(fcb->name, ".") == 0 || strcmp(fcb->name, "..") == 0) {
            printf("[Rename] You can't rename %s\n", src);
            return -1;
        }
        strcpy(fcb->name, dst);
    }
    else {
        printf("[Rename] Not found %s\n", src);
        return -1;
    }
    return 0;
}
//open
int Open(char* path)
{
    Fcb* fcb = searchFcb(path, open_path[current]);
    if (fcb) {
        if (fcb->is_dir != 0) {
            printf("[Open] %s is not readable file\n", fcb->name); return -1;
        }
        //��ȡ�ź���
        char mutex_name[256];
        getAbsPath(path, mutex_name);
        char* suffix = mutex_name + strlen(mutex_name);
        //����Ƿ���д����
        strcpy(suffix, "-write");
        sem_write = sem_open(mutex_name, O_CREAT, 0666, 1);
        int sval;
        sem_getvalue(sem_write, &sval);
        if (sval < 1) {
            printf("[Open] %s is busy\n", fcb->name); return -1;
        }
        //���������̣���ȡ�ļ����������ź���
        strcpy(suffix, "-read");
        sem_read = sem_open(mutex_name, O_CREAT, 0666, READ_MAX);
        sem_wait(sem_read);
        //������ڸ��ļ������ȡ�ļ�����
        char* p = (char*)getBlock(fcb->start_block);
        for (int i = 0; i < fcb->size; i++) {
            printf("%c", *p);
            p++;
        }
        printf("\n");
        getchar();
        printf("[Open] input the enter to return!");
        getchar();
        //���Ӷ��ź���
        sem_post(sem_read);
    }
    //�ļ������ڣ������ļ�
    else {
        Fcb* parent = getParent(path);
        if (parent == NULL) {
            printf("[Open] Not found %s\n", path); return -1;
        }
        Fcb* fcb = getFreeFcb(parent);
        char* name = getPathLastName(path);
        createFcb(fcb, name, 0, sizeof(Block));
        fcb->size = 0;
        parent->size += sizeof(Fcb);
    }
    return 0;
}

// write file
int Write(char* path)
{
    Fcb* fcb = searchFcb(path, open_path[current]);
    if (fcb) {
        if (fcb->is_dir != 0) {
            printf("[Write] %s is not writable file\n", fcb->name);
            return -1;
        }
        //��ȡ�ź���
        char mutex_name[256];
        getAbsPath(path, mutex_name);
        char* suffix = mutex_name + strlen(mutex_name);
        //����Ƿ��н����ڶ�
        strcpy(suffix, "-read");
        sem_read = sem_open(mutex_name, O_CREAT, 0666, READ_MAX);
        int sval;
        sem_getvalue(sem_read, &sval);
        //�н����ڶ�
        if (sval < READ_MAX) {
            printf("[Write] %s is busy\n", fcb->name);
            return -1;
        }
        //����д����
        strcpy(suffix, "-write");
        sem_write = sem_open(mutex_name, O_CREAT, 0666, 1);
       //������������д
        sem_getvalue(sem_write, &sval);
        if (sval < 1) {
            printf("[Write] Waiting for other users...\n");
        }
        //����д�����ź���
        sem_wait(sem_write);
        printf("[Write] You can write now\n");
        //���ڸ��ļ���������д���ļ�����
        char* head = (char*)(disk + fcb->start_block);
        char* p = head;
        getchar(); //ȥ����������Ļس�
        while ((*p = getchar()) != 27 && *p != EOF) {
            p++;
        }
        *p = 0;
        fcb->size = strlen(head);
        //�����ź���
        sem_post(sem_write);
    }
    //�������ļ�
    else {
        printf("[Write] Not found %s\n", path);
    }
    return 0;
}

// rm file
int Rm(char* path, Fcb* root)
{
    Fcb* fcb = searchFcb(path, root);
    if (fcb) {
        if (fcb->is_dir != 0) {
            printf("[Rm] %s is not file\n", fcb->name); return -1;
        }
        // �ͷ�ʹ�õ��Ŀ�
        for (int i = 0; i < getBlockNum(fcb->size); i++) {
            fat[fcb->start_block + i] = FREE;
        }
        fcb->is_used = 0;
        //�ı丸�ڵ�Ĵ�С��ֵ
        getParent(path)->size -= sizeof(Fcb);
    }
    else {
        printf("[Rm] Not found %s\n", path); return -1;
    }
    return 0;
}

// ls
void Ls()
{
    Fcb* fcb = open_path[current];
    int num = open_path[current]->size / sizeof(Fcb);
    for (int i = 0; i < FCB_LIST_LEN; i++) {
        if (fcb->is_used) {
            printf("%s\t", fcb->name);
        }
        fcb++;
    }
    printf("\n");
}

// ls -l
void Lls()
{
    Fcb* fcb = open_path[current];
    int num = open_path[current]->size / sizeof(Fcb);
    for (int i = 0; i < 2; i++) {
        if (fcb->is_used) {
            printf("%s\n", fcb->name);
        }
        fcb++;
    }
    for (int i = 2; i < FCB_LIST_LEN; i++) {
        if (fcb->is_used) {
            printf("%hu-%hu-%hu %hu:%hu:%hu\t", fcb->datetime.year, fcb->datetime.month, fcb->datetime.day, fcb->datetime.hour, fcb->datetime.minute, fcb->datetime.second);
            printf("Block %hd  \t", fcb->start_block);
            printf("%hu B\t", fcb->size);
            printf("%s\t", fcb->is_dir ? "Dir" : "File");
            printf("%s\n", fcb->name);
        }
        fcb++;
    }
}

int Cd(char* path)
{
    char* names[16];
    int len = split(names, path, "/");
    for (int i = 0; i < len; i++) {
        if (strcmp(names[i], ".") == 0) continue;
        if (strcmp(names[i], "..") == 0) {
            if (current == 0) {
                printf("[Cd] Depth of the directory has reached the lower limit\n");
                return -1;
            }
            current--; continue;
        }
        if (current == 15) {
            printf("[Cd] Depth of the directory has reached the upper limit\n"); return -1;
        }
        //�ж�Ŀ¼�Ƿ����
        Fcb* fcb = searchFcb(names[i], open_path[current]);
        if (fcb) {
            if (fcb->is_dir != 1) {
                printf("[Cd] %s is not directory\n", names[i]); return -1;
            }
            current++;
            open_name[current] = fcb->name;
            open_path[current] = (Fcb*)getBlock(fcb->start_block);
        }
        else {
            printf("[Cd] %s is not existed\n", names[i]); return -1;
        }
    }
    return 0;
}

int Help()
{
    printf("\n");
    printf("help\toutput help\n");
    printf("ls\toutput dir information\n");
    printf("lls\toutput dir in long format\n");
    printf("cd\tmove to other path \n");
    printf("mkdir\tcreate dir\n");
    printf("rmdir\trm dir recursively\n");
    printf("open\topen file, if it not exit, create a new one\n");
    printf("rm\trm file\n");
    printf("rename\talter name\n");
    printf("exit\texit file-sys\n");
    printf("\n");
}
/*------------------------------------�ļ���������---------------------------------------*/

/*--------------------------------------������������-------------------------------------*/
// �����shell����ʾ��
void printPathInfo()
{
    printf("MMH@FileSystem:");
    for (int i = 0; i <= current; i++) {
        printf("/%s", open_name[i]);
    }
    printf("> ");
}
int split(char** arr, char* str, const char* delims)
{
    int count = 0;
    char _str[64];
    strcpy(_str, str);
    char* s = strtok(_str, delims);
    while (s != NULL) {
        count++;
        *arr++ = s;
        s = strtok(NULL, delims);
    }
    return count;
}

// ������ȡ
char* getArg(char* str)
{
    scanf("%s", str);
    return str;
}

// ��������
char* getCmd(char* cmd)
{
    scanf("%s", cmd);
    return cmd;
}
/*--------------------------------------������������-------------------------------------*/