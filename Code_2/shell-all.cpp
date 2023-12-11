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
    char buf[256], * buf2, * cmd, * cmd2, * vec[128], * vec2[128], * infile, * outfile, * infile2, * outfile2;
    int n, sv, pfd[2];
    while (1)
    {
        print_prompt();
        if (fgets(buf, sizeof(buf), stdin) == NULL)
        {
            exit(0);
        }
        char* tmp;//替换换行符
        while (tmp = strstr(buf, "\n")) {
            *tmp = ' ';
        }
        //处理管道符号
        buf2 = strstr(buf, "|");
        cmd2 = NULL;
        if (buf2)
        {
            *buf2++ = 0;
            //处理重定向符号
            infile2 = strstr(buf2, "<");
            outfile2 = strstr(buf2, ">");
            if (infile2)
            {
                *infile2 = 0;
                infile2 = strtok(infile2 + 1, " ");
            }
            if (outfile2)
            {
                *outfile2 = 0;
                outfile2 = strtok(outfile2 + 1, " ");
            }
            //处理命令参数
            cmd2 = strtok(buf2, " ");
            if (!cmd2)
            {
                exit(1);
            }
            n = 0;
            vec2[n++] = cmd2;
            while (vec2[n++] = strtok(NULL, " "));
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
        //处理了命令参数
        cmd = strtok(buf, " ");
        if (cmd)
        {
            if (strcmp(cmd, "exit") == 0)
            {
                exit(0);
            }
            n = 0;
            vec[n++] = cmd;
            while (vec[n++] = strtok(NULL, " "));
            //到此为止两边的参数准备完成
            //创建管道
            pipe(pfd);
            if (fork() == 0)
            {
                //处理重定向的问题
                int fd0 = -1, fd1 = -1;
                //首先测试重定向文件是否打开成功
                if (infile)
                {
                    fd0 = open(infile, O_RDONLY);
                }
                //如果打开成功则进行重定向
                if (fd0 != -1)
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
                if (fd1 != -1)
                {
                    dup2(fd1, 1);
                }
                close(fd1);
                //看cmd2命令是否存在，存在就需要使用管道，把输出写入管道
                if (cmd2)
                {
                    dup2(pfd[1], 1);
                }
                close(pfd[0]);
                close(pfd[1]);
                execvp(cmd, vec);
                fprintf(stderr, "**ERROR:%s\n", strerror(errno));
                exit(1);
            }//如果cmd2存在则再创建一个子进程
            else if (cmd2 && fork() == 0)
            {
                //这边执行的是管道右边的命令
                int fd0 = -1, fd1 = -1;
                //这里的输入只可能是来自己于管道另一端
                dup2(pfd[0], 0);
                close(pfd[0]);
                close(pfd[1]);
                //输出还是有可能输出重定向的
                //测试重定向文件是否打开成功
                if (outfile2)
                {
                    fd1 = open(outfile2, O_CREAT | O_WRONLY, 0666);
                }
                //打开成功就重定向
                if (fd1 != -1)
                {
                    dup2(fd1, 1);
                }
                close(fd1);
                execvp(cmd2, vec2);
                printf("fail\n");
                exit(1);
            }
            close(pfd[0]);
            close(pfd[1]);
            wait(&sv);
            // 			if (cmd2)wait(&sv);
            wait(&sv);
        }
    }
}
int main() {
    shell();
    return 0;
}

