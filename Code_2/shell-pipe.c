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
    printf("[%s@%s %s]$", user_name, host_name, path);
}
void shell() {
    char buf[256], * buf2, * cmd, * cmd2, * vec[128], * vec2[128];
    int n, sv, pfd[2];
    while (1)
    {
        print_prompt();
        if (fgets(buf,sizeof(buf),stdin)==NULL)
        {
            exit(0);
        }
        char* tmp;//替换换行符为tmp
        while(tmp=strstr(buf,"\n")){
            *tmp=' ';
        }
        //处理管道分隔符，并记地址位置为buf
        buf2 = strstr(buf, "|");
        if (!buf2)//没找到分隔符，退出
        {
            exit(0);
        }
        *buf2++ = 0;
        //处理管道右边命令
        cmd2 = strtok(buf2, " ");
        if (!cmd2)
        {
            exit(1);
        }
        n = 0;
        vec2[n++] = cmd2;
        while (vec2[n++] = strtok(NULL, " "));
        //处理管道左边的命令
        cmd = strtok(buf, " ");
        if (!cmd)
        {
            exit(1);
        }
        n = 0;
        vec[n++] = cmd;
        while (vec[n++] = strtok(NULL, " "));

        pipe(pfd);//创建管道
        if (fork()==0)
        {
            //子进程运行：
            dup2(pfd[0], 0);
            close(pfd[0]);
            close(pfd[1]);
            execvp(cmd2, vec2);
            exit(1);
        }
        else if (fork()==0)
        {
            dup2(pfd[1], 1);
            close(pfd[0]);
            close(pfd[1]);
            execvp(cmd, vec);
            exit(1);
        }
        close(pfd[0]);
        close(pfd[1]);
        wait(&sv);
        wait(&sv);
    }
}
int main(){
    shell();
    return 0;
}