#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pwd.h>
#include<errno.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
void print_prompt() {
    char host_name[] = "localhost";
    char* user_name;
    char path[48];
    struct passwd* pwd;
    pwd = getpwuid(getuid());
    user_name = pwd->pw_name;
    getcwd(path, 48);
    //打印终端样式
    printf("[%s@%s %s]$", user_name, host_name, path);
}
void shell() {
    char buf[256], * cmd, * vec[128];
    int n, sv;
    while (1)
    {
        //首先打印终端
        print_prompt();
        //获取键盘输入语句
        if (fgets(buf,sizeof(buf),stdin)==NULL)
        {
            exit(0);
        }
        char* tmp;//用于替换换行符
        while(tmp=strstr(buf,"\n")){
            *tmp=' ';
        }
        //空格切分后得到的命令
        cmd = strtok(buf, " ");//通过空格切分
        if (cmd)
        {
            if (strcmp(cmd,"exit")==0)
            {
                exit(0);
            }
            n = 0;
            vec[n++] = cmd;
            while (vec[n++] = strtok(NULL, " "));//获取通过空格切分的参数
            if (fork()==0)
            {
                //创建子进程，子进程执行execvp()
                execvp(cmd, vec);//执行结果
                fprintf(stderr, "**ERROR:%s\n", strerror(errno));
                exit(1);
            }
            wait(&sv);
        }
    }
}
int main() {
    shell();
    return 0;
}

