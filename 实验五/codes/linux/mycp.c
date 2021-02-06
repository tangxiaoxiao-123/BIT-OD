#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <utime.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
const int MAXLENTH = 512;
const int BUF_SIZE = 4096;
void copyfile(const char *source, const char *target)
{
    struct stat statBuf;
    int fds, fdt, readSize;
    lstat(source, &statBuf);
    if ((fds = open(source, O_RDONLY)) == -1)
    {
        printf("open file error!\n");
        exit(0);
    }
    if ((fdt = creat(target, statBuf.st_mode)) == -1)
    {
        printf("create file error!\n");
        exit(0);
    }
    char buffer[BUF_SIZE];
    while ((readSize = read(fds, buffer, BUF_SIZE)) > 0)
    {
        int isok = write(fdt, buffer, readSize);
        if (isok != readSize)
        {
            printf("write error!\n");
            exit(0);
        }
        memset(buffer, 0, sizeof(buffer));
    }
    close(fds);
    close(fdt);
}

void mycp(const char *source, const char *target)
{
    char sourcePath[MAXLENTH];
    char targetPath[MAXLENTH];
    DIR *dir;
    struct dirent *entry;
    struct stat statBuf;
    struct utimbuf timebuf;

    //打开源文件目录
    if ((dir = opendir(source)) != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            //获得源文件名和目标文件名
            memset(sourcePath, 0, sizeof(sourcePath));
            memset(targetPath, 0, sizeof(targetPath));
            strcat(sourcePath, source);
            strcat(sourcePath, "/");
            strcat(sourcePath, entry->d_name);

            strcat(targetPath, target);
            strcat(targetPath, "/");
            strcat(targetPath, entry->d_name);

            lstat(sourcePath, &statBuf);

            //软链接文件
            if (S_ISLNK(statBuf.st_mode))
            {
                char buffer[BUF_SIZE];
                //读取文件内容
                int len = readlink(sourcePath, buffer, BUF_SIZE);
                buffer[len] = '\0';
                //创建软链接文件
                symlink(buffer, targetPath);
                //修改时间
                struct timeval times[2];
                times[0].tv_sec = statBuf.st_atime; //访问时间
                times[0].tv_usec = 0;
                times[1].tv_sec = statBuf.st_mtime; //修改时间
                times[1].tv_usec = 0;
                lutimes(targetPath, times);
            }
            else
            {
                //目录文件
                if (S_ISDIR(statBuf.st_mode))
                {
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    {
                        continue;
                    }
                    mkdir(targetPath, statBuf.st_mode);
                    mycp(sourcePath, targetPath);
                }
                else
                {
                    copyfile(sourcePath, targetPath);
                }
                //修改时间
                timebuf.actime = statBuf.st_atime;  //访问时间
                timebuf.modtime = statBuf.st_mtime; //修改时间
                utime(targetPath, &timebuf);        //修改时间
            }
            printf("copy from %s to %s\n", sourcePath, targetPath);
        }
        closedir(dir);
    }
    else
    {
        printf("open source file error!\n");
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("parameter number must be 3!\n");
        printf("usage:mycp sourcePath targetPath\n");
        exit(0);
    }
    printf("Start copying!\n");
    struct stat statBuf;
    struct utimbuf timebuf;

    DIR *dir = opendir(argv[1]);
    if (dir == NULL)
    {
        printf("open source file error!\n");
        exit(0);
    }
    //获取源文件信息
    lstat(argv[1], &statBuf);
    closedir(dir);

    //目标目录文件
    dir = opendir(argv[2]);
    if (dir == NULL)
    {
        //创建目录
        mkdir(argv[2], statBuf.st_mode);

        //复制文件
        mycp(argv[1], argv[2]);

        //修改目录创建时间
        timebuf.actime = statBuf.st_atime;  //访问时间
        timebuf.modtime = statBuf.st_mtime; //修改时间

        utime(argv[2], &timebuf); //修改时间

        printf("Copy success!\n");
    }
    return 0;
}
