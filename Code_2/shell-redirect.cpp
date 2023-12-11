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
    char buf[256], * cmd, * vec[128], * infile, * outfile;
    int n, sv;
    while (1)
    {
        print_prompt();
        if (fgets(buf,sizeof(buf),stdin)==NULL)
        {
            exit(0);
        }
        char* tmp;//替换换行符
        while(tmp=strstr(buf,"\n")){
            *tmp=' ';
        }
        //处理重定向符号
        infile = strstr(buf, "<");
        outfile = strstr(buf, ">");
        //获取重定向的文件
        if (infile)
        {
            *infile = 0;
            infile = strtok(infile + 1, " ");
        }
        if (outfile)
        {
            *outfile = 0;
            outfile = strtok(outfile + 1, " ");
        }
        cmd = strtok(buf, " ");
        if (cmd)
        {
            if (strcmp(cmd,"exit")==0)
            {
                exit(0);
            }
            n = 0;
            vec[n++] = cmd;
            while (vec[n++] = strtok(NULL, " "));
            if (fork()==0)
            {
                int fd0 = -1, fd1 = -1;
                //首先测试重定向文件是否打开成功
                if (infile)
                {
                    fd0 = open(infile, O_RDONLY);
                }
                //如果打开成功则进行重定向
                if (fd0!=-1)
                {
                    dup2(fd0, 0);
                }
                close(fd0);
                //测试重定向文件是否打开成功
                if (outfile)
                {
                    fd1 = open(outfile, O_CREAT | O_WRONLY, 0666);
                }
                //打开成功就重定向
                if (fd1!=-1)
                {
                    dup2(fd1, 1);
                }
                close(fd1);
                execvp(cmd, vec);
                fprintf(stderr, "**ERROR:%s\n", strerror(errno));
                exit(1);
            }
            wait(&sv);
        }
    }
}
int main(){
    shell();
    return 0;
}
