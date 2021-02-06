#include <Windows.h>
#include <stdio.h>
#include<iostream>
using namespace std;

//得到当前时间
long int GetTime()
{
    SYSTEMTIME time;
    ZeroMemory(&time, sizeof(time));
    GetSystemTime(&time);
    //输出时间
    cout<<time.wYear<<"/"<<time.wMonth<<"/"<<time.wDay<<", "<<time.wHour+8
    <<"h:"<<time.wMinute<<"m:"<<time.wSecond<<"s:"<<time.wMilliseconds<<"ms"<<endl;
    //得到以毫秒为单位的时间结果方便做减法
    long int resTime = time.wMinute*60*1000+time.wSecond*1000+time.wMilliseconds;
    return resTime;
}

    
void Parent(char *clonename)
{

    //格式化子进程的命令行
    TCHAR szCmdLine[MAX_PATH];
    sprintf(szCmdLine, "%s", clonename);

    //子进程启动信息
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    //子进程的有关信息
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    //子进程开始时间
    cout<<"Start time: ";
    long int startTime = GetTime();

    //创建子进程
    BOOL bCreateOK = CreateProcess(
        NULL,//可执行的应用程序名称（这里可以为NULL）
        szCmdLine,//要执行的命令行
        NULL,//缺省的进程安全性
        NULL,//缺省的线程安全性
        FALSE,//不继承句柄
        CREATE_NEW_CONSOLE,//创建新的控制台
        NULL,//使用父进程的环境块
        NULL,//使用父进程的工作目录
        &si,//启动信息
        &pi);//返回新进程和线程的结构信息
    if (bCreateOK)
    {
        printf("Create ChildProcess Succuss!\n");
        WaitForSingleObject(pi.hProcess, INFINITE);

        //子进程结束时间
        cout<<"End time: ";
        long int endTime = GetTime();
        printf("the interval time:%lld\n", endTime - startTime);

        //关闭句柄，防止资源浪费
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        printf("Create ChildProcess Failed.\n");
    }
    
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "ChildProcess") == 0)
    {
        
        Parent(argv[1]);
    }
    else
    {
        printf("Not Create ChildProcess");
    }
    
    return 0;
}

