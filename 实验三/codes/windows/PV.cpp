#include <windows.h>
#include <stdio.h>
#include <time.h>
struct buf
{
    char text[4];
    int read_op;
    int write_op;
};
const char dic[3] = {'T', 'X', 'J'};
HANDLE lphandles[7] = {NULL};
HANDLE lthandles[7] = {NULL};
int cnt = 0;

#define MUTEX "Mutex"
#define EMPTY "Empty"
#define FULL "Full"

void createChildProcess(const char *name)
{
    //得到进程所在文件的名字
    TCHAR szFilename[MAX_PATH];
    GetModuleFileName(NULL, szFilename, MAX_PATH);

    TCHAR szCmdLine[MAX_PATH];
    sprintf(szCmdLine, "\"%s\" %s", szFilename, name);
    //printf("%s\n", szCmdLine);

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    BOOL bCreateOK = CreateProcess(
        NULL,      //可执行的应用程序名称（这里可以为NULL）
        szCmdLine, //要执行的命令行
        NULL,      //缺省的进程安全性
        NULL,      //缺省的线程安全性
        FALSE,     //不继承句柄
        0,         //继承父进程的控制台
        NULL,      //使用父进程的环境块
        NULL,      //使用父进程的工作目录
        &si,       //启动信息
        &pi);      //返回新进程和线程的结构信息
    if (bCreateOK)
    {
        lphandles[cnt] = pi.hProcess;
        lthandles[cnt] = pi.hThread;
        cnt++;
    }
    else
    {
        printf("create process error!\n");
        exit(0);
    }
}
void Producer()
{
    //printf("I am a Producer.\n");
    HANDLE empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, true, EMPTY);
    HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, false, MUTEX);
    HANDLE full = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, FULL);
    if (!(empty && mutex && full))
    {
        printf("producer open handle error!\n");
        exit(0);
    }
    for (int i = 0; i < 4; i++)
    {
        Sleep(rand() % 3000 + 1000);

        WaitForSingleObject(empty, INFINITE); //P(empty)
        WaitForSingleObject(mutex, INFINITE); //P(mutex)
        //printf("write something.\n");
        //向缓冲区写
        HANDLE buffer_map = OpenFileMapping(FILE_MAP_ALL_ACCESS, true, "Buffer");
        if(!buffer_map)
        {
            printf("%d\n",GetLastError());
            printf("producer open file_map error!\n");
            exit(0);
        }
        LPVOID addr = MapViewOfFile(buffer_map, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        struct buf *buffer = (struct buf *)addr;
        int od = rand() % 3;
        buffer->text[buffer->write_op] = dic[od];
        buffer->write_op = (buffer->write_op + 1) % 4;
        //输出相应内容
        printf("the producer: push %c\n", dic[od]);
        printf("the buffer-text:");
        for (int i = 0; i < 4; i++)
        {
            printf("%c", buffer->text[i]);
        }
        printf("\n");
        //取消映射
        UnmapViewOfFile(addr);

        ReleaseMutex(mutex);             //V(mutex)
        ReleaseSemaphore(full, 1, NULL); //V(full)
        //关闭句柄
        CloseHandle(buffer_map);
    }
    CloseHandle(mutex);
    CloseHandle(empty);
    CloseHandle(full);
}
void Consumer()
{
    //printf("I am a Consumer.\n");
    HANDLE empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, EMPTY);
    HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX);
    HANDLE full = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, FULL);
    if (!(empty && mutex && full))
    {
        printf("consumer open handle error!\n");
        exit(0);
    }
    for (int i = 0; i < 3; i++)
    {
        Sleep(rand() % 3000 + 1);

        WaitForSingleObject(full, INFINITE);  //P(full)
        WaitForSingleObject(mutex, INFINITE); //P(mutex)
        //printf("waiting\n");

        //向缓冲区写
        HANDLE buffer_map = OpenFileMapping(FILE_MAP_ALL_ACCESS, true, "Buffer");

        if(!buffer_map)
        {
            printf("consumer open file_map error!\n");
            exit(0);
        }
        LPVOID addr = MapViewOfFile(buffer_map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        CloseHandle(buffer_map);
        struct buf *buffer = (struct buf *)addr;
        char c = buffer->text[buffer->read_op];
        buffer->text[buffer->read_op] = '-';
        buffer->read_op = (buffer->read_op + 1) % 4;
        //输出
        printf("the consumer : pop %c\n", c);
        printf("the buffer-text:");
        for (int i = 0; i < 4; i++)
        {
            printf("%c", buffer->text[i]);
        }
        printf("\n");
        //取消映射
        UnmapViewOfFile(addr);
        addr = NULL;
        ReleaseMutex(mutex);              //V(mutex)
        ReleaseSemaphore(empty, 1, NULL); //V(empty)
        
    }
    //关闭句柄

    CloseHandle(mutex);
    CloseHandle(empty);
    CloseHandle(full);
    
}
void Parent()
{
    //创建互斥体(缺省的安全性，创建进程就有该互斥体，互斥体的名字)
    HANDLE mutex = CreateMutex(NULL, true, MUTEX);

    //创建信号量
    HANDLE full = CreateSemaphore(NULL, 0, 4, FULL);

    HANDLE empty = CreateSemaphore(NULL, 4, 4, EMPTY);

    if (!(empty && full && mutex))
    {
        printf("create handles error!\n");
    }
    //创建文件映射对象
    HANDLE buffer_map = CreateFileMapping(
        INVALID_HANDLE_VALUE, //与物理文件无关
        NULL,                 //子进程不继承句柄
        PAGE_READWRITE,
        0,
        sizeof(buf),
        "Buffer");
    if(!buffer_map)
    {
        printf("create file_map error!\n");
        exit(0);
    }
    LPVOID addr = MapViewOfFile(
        buffer_map,          //句柄
        FILE_MAP_ALL_ACCESS, //保护模式
        0,                   //偏移量的低字节
        0,                   //偏移量的高字节
        0                    //映射文件的真实大小
    );
    if(addr == NULL)
    {
        printf("mapping error!\n");
        exit(0);
    }
    struct buf *buffer = (struct buf *)addr;
    buffer->write_op = 0;
    buffer->read_op = 0;
    for (int i = 0; i < 4; i++)
        buffer->text[i] = '-';
    UnmapViewOfFile(addr);
    addr = NULL;
    for (int i = 0; i < 3; i++)
    {
        createChildProcess("Producer");
    }
    for (int i = 0; i < 4; i++)
    {
        createChildProcess("Consumer");
    }
    ReleaseMutex(mutex);
    WaitForMultipleObjects(cnt, lphandles, true, INFINITE);
    for (int i = 0; i < cnt; i++)
    {
        CloseHandle(lphandles[i]);
        CloseHandle(lthandles[i]);
    }
    
    CloseHandle(mutex);
    CloseHandle(full);
    CloseHandle(empty);
    CloseHandle(buffer_map);
}
int main(int argv, char *argc[])
{
    srand(clock());
    if (argv > 1)
    {

        if (strcmp(argc[1], "Producer") == 0)
        {
            //生产者执行
            Producer();
        }
        else if (strcmp(argc[1], "Consumer") == 0)
        {
            //消费者执行
            Consumer();
        }
    }
    else
    {
        Parent();
        //system("pause");
    }

    return 0;
}