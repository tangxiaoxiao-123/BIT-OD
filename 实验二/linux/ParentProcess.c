#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
int main(int argc, char *argv[])
{
    pid_t pid;
    struct timeval tv_Start, tv_End;
    struct timezone tz_Start, tz_End;

    long int start_time;
    long int end_time;

    //获取可执行文件的路径
    char path[80] = "/home/ostxj/studycodes/";
    strcat(path, argv[1]);

    //获取子进程开始时间
    gettimeofday(&tv_Start, NULL);
    start_time = tv_Start.tv_sec * 1000 + tv_Start.tv_usec / 1000;
    printf("The ChildProcess starts %ld.\n", start_time);

    if ((pid = fork()) < 0)
    {
        printf("fork error.\n");
        exit(-1);
    }

    else if (pid == 0)
    {
        //子进程在执行

        printf("The childProcess is processing.\n");
        if (execv(path, NULL) < 0)
        {
            printf("execv error!\n");
            exit(-1);
        }
    }
    else
    {

        //父进程执行
        wait(0);

        //子进程结束，获取子进程结束时间
        gettimeofday(&tv_End, NULL);
        end_time = tv_End.tv_sec * 1000 + tv_End.tv_usec / 1000;
        printf("The ChildProcess ends %ld.\n", end_time);

        printf("the childprocess interval %ld.\n", end_time - start_time);
    }

    exit(0);
}
