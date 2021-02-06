#include <windows.h>
#include <iostream>
#include<Shlwapi.h>
#include <psapi.h> 
#include<iomanip>
#include<TlHelp32.h>

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"kernel32.lib")
using namespace std;

inline bool TestSet(DWORD dwTarget, DWORD dwMask)
{
    return ((dwTarget & dwMask) == dwMask);
}

#define SHOWMASK(dwTarget,type)\
if(TestSet(dwTarget,PAGE_##type))\
{printf("%20s",#type);return;}
//{cout << #type;}


void showSystemInfo()
{
    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(si));

    GetSystemInfo(&si);

    cout << "*****************系统内存地址空间布局*****************" << endl;
    cout << "内存页的大小：" << si.dwPageSize / 1024 << "KB" << endl;
    cout << "进程可用最小地址：" << si.lpMinimumApplicationAddress << endl;
    cout << "进程可用最大地址：" << si.lpMaximumApplicationAddress << endl;
    cout << "处理机数量：" << si.dwNumberOfProcessors << endl;
    cout << "虚拟内存的粒度：" << si.dwAllocationGranularity * 1.0 / 1024 << "KB" << endl;
    cout << endl;

}

void showMemoryPhysInfo()
{
    MEMORYSTATUSEX memPhys;
    ZeroMemory(&memPhys, sizeof(memPhys));
    memPhys.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&memPhys))
    {
        cout << "失败！" << endl;
        return;
    }
    cout << "*****************系统物理内存信息*****************" << endl;

    cout << "物理内存总量：" << (memPhys.ullTotalPhys * 1.0 / (1024 * 1024 * 1024)) << "GB" << endl;
    cout << "物理内存使用率：" << memPhys.dwMemoryLoad << "%" << endl;
    cout << "物理内存使用量：" << ((memPhys.ullTotalPhys - memPhys.ullAvailPhys) * 1.0 / (1024 * 1024 * 1024)) << "GB" << endl;
    cout << "虚拟内存总量：" << (memPhys.ullTotalVirtual * 1.0 / (1024 * 1024 * 1024)) << "GB" << endl;
    cout << "虚拟内存使用量：" << ((memPhys.ullTotalVirtual - memPhys.ullAvailVirtual) * 1.0 / (1024 * 1024)) << "MB" << endl;

    PERFORMANCE_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    pi.cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pi, sizeof(pi));

    cout << "系统当前提交的页面总数：" << pi.CommitTotal << "页" << endl;
    cout << "系统缓存内存量：" << pi.SystemCache << "页" << endl;
    cout << "当前分页和非分页内核池中的内存总和：" << pi.KernelTotal << "页" << endl;
    cout << "当前分页内核池中的内存：" << pi.KernelPaged << "页" << endl;
    cout << "当前在非分页内核池中的内存：" << pi.KernelNonpaged << "页" << endl;
    cout << "打开句柄数：" << pi.HandleCount << endl;
    cout << "当前进程数：" << pi.ProcessCount << endl;
    cout << "当前线程数：" << pi.ThreadCount << endl;
    cout << endl;

}

void AllProcessID()
{
    cout << "*****************查看所有进程的相关信息*****************" << endl;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout << "CreateToolHelp32Snapshot 调用失败" << endl;
        return ;
    }
    PROCESSENTRY32 pe32;
    ZeroMemory(&pe32, sizeof(pe32));
    pe32.dwSize = sizeof(pe32);
    BOOL bMore = Process32First(hProcessSnap, &pe32);
    if (!bMore)
    {
        cout << "Process32First 调用失败！" << endl;
    }
    int num = 0;
    char head[10][10] = { "序号","PID","名称","优先级" };
    printf("%5s %6s %80s %5s\n",head[0],head[1],head[2],head[3]);
    while (bMore)
    {

        printf("%5d %6d %80ls %5d\n", ++num,pe32.th32ProcessID, pe32.szExeFile,pe32.pcPriClassBase);
        bMore = Process32Next(hProcessSnap, &pe32);
    }
    printf("\n");
    return;
}
void showProtection(DWORD dwTarget)
{
    SHOWMASK(dwTarget, READONLY);
    SHOWMASK(dwTarget, GUARD);
    SHOWMASK(dwTarget, NOCACHE);
    SHOWMASK(dwTarget, READWRITE);
    SHOWMASK(dwTarget, WRITECOPY);
    SHOWMASK(dwTarget, EXECUTE);
    SHOWMASK(dwTarget, EXECUTE_READ);
    SHOWMASK(dwTarget, EXECUTE_READWRITE);
    SHOWMASK(dwTarget, EXECUTE_WRITECOPY);
    SHOWMASK(dwTarget, NOACCESS);

}
void showProcessInfo(DWORD pid)
{
    cout << "*****************查看特定进程的信息*****************" << endl;
    //获得进程句柄
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    //当前进程的工作集大小
    PROCESS_MEMORY_COUNTERS pmc;
    ZeroMemory(&pmc, sizeof(pmc));
    pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
    BOOL bt = GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
    if (bt)
        cout << "当前进程的工作集大小：" << pmc.WorkingSetSize * 1.0 / (1024 * 1024) << "MB" << endl;
    else
    {
        cout << "查看进程工作集失败！" << endl;
        return;
    }
        

    MEMORY_BASIC_INFORMATION mbi;
    ZeroMemory(&mbi, sizeof(mbi));


    //hProcess = GetCurrentProcess();
    //获得进程可用的地址空间
    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(si));
    GetSystemInfo(&si);
    LPVOID pBlock = si.lpMinimumApplicationAddress;

    char header[10][10] = { "块的范围","状态","保护类型","文件类型","模块名" };
    printf("%26s %15s %20s %10s %30s\n", header[0], header[1], header[2], header[3],header[4]);
    while (pBlock < si.lpMaximumApplicationAddress)
    {
        if (VirtualQueryEx(hProcess, pBlock, &mbi, sizeof(mbi)) == sizeof(mbi))
        {
            //计算块的结尾和长度
            LPVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;

            TCHAR szSize[MAX_PATH];
            StrFormatByteSize(mbi.RegionSize, szSize, MAX_PATH);

            printf("%08x-%08x (%8ls) ", pBlock, pEnd, szSize);
            /*cout.fill('0');
            cout << hex << setw(8) << (DWORD)pBlock
                << "-"
                << hex << setw(8) << (DWORD)pEnd
                << "("<<szSize<<")   ";*/

            //显示块的状态
            switch (mbi.State)
            {
            case MEM_COMMIT:
                printf("%15s", "提交");
                //cout << "提交   ";
                break;
            case MEM_FREE:
                printf("%15s", "空闲");
                //cout << "空闲   ";
                break;
            case MEM_RESERVE:
                printf("%15s", "保留");
                //cout << "保留   ";
                break;
            }

            //显示保护

            if (mbi.Protect == 0 && mbi.State != MEM_FREE) {
                mbi.Protect = PAGE_READONLY;
            }
            showProtection(mbi.Protect);

            //显示类型
            switch (mbi.Type)
            {
            case MEM_IMAGE:
                printf("%10s ", "Image");
                //cout << ",Image";//映像文件
                break;
            case MEM_MAPPED:
                printf("%10s ", "Mapped");
                //cout << ",Mapped";//内存映射文件
                break;
            case MEM_PRIVATE:
                printf("%10s ", "Private");
                //cout << ",Private";//私有内存区
                break;
            }

           
            //检验可执行的映像
            TCHAR fileName[MAX_PATH];

            if (GetModuleFileName(
                (HMODULE)pBlock,
                fileName,
                MAX_PATH
            ) > 0)
            {
                //除去路径显示
                PathStripPath(fileName);
                printf("%30ls", fileName);
                
            }

            cout << endl;
            pBlock = pEnd;
        }
    }

}
int main()
{
    cout << "*****************内存监视器*****************" << endl;
    cout << "序号对应的功能：" << endl;
    cout << "1. 系统内存地址空间布局" << endl;
    cout << "2. 系统物理内存信息" << endl;
    cout << "3. 查看所有进程" << endl;
    cout << "4. 查看特定进程的信息" << endl;
    cout << "0. 退出" << endl;
    while (1)
    {
        cout << "请输入序号：";
        int n;
        cin >> n;
        switch (n)
        {
        case 1:
            showSystemInfo();
            cout << "显示完毕！" << endl;
            break;
        case 2:
            showMemoryPhysInfo();
            cout << "显示完毕！" << endl;
            break;
        case 3:
            AllProcessID();
            cout << "显示完毕！" << endl;
            break;
        case 4:
            DWORD pid;
            cout << "请输入进程号pid：";
            cin >> pid;
            showProcessInfo(pid);
            cout << "显示完毕！" << endl;
            break;
        case 0:
            return 0;
        default:
            cout << "输入不符合格式" << endl;
            break;
        }

        
    }
    

    
    return 0;
}