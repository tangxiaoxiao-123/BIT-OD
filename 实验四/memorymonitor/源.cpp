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

    cout << "*****************ϵͳ�ڴ��ַ�ռ䲼��*****************" << endl;
    cout << "�ڴ�ҳ�Ĵ�С��" << si.dwPageSize / 1024 << "KB" << endl;
    cout << "���̿�����С��ַ��" << si.lpMinimumApplicationAddress << endl;
    cout << "���̿�������ַ��" << si.lpMaximumApplicationAddress << endl;
    cout << "�����������" << si.dwNumberOfProcessors << endl;
    cout << "�����ڴ�����ȣ�" << si.dwAllocationGranularity * 1.0 / 1024 << "KB" << endl;
    cout << endl;

}

void showMemoryPhysInfo()
{
    MEMORYSTATUSEX memPhys;
    ZeroMemory(&memPhys, sizeof(memPhys));
    memPhys.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&memPhys))
    {
        cout << "ʧ�ܣ�" << endl;
        return;
    }
    cout << "*****************ϵͳ�����ڴ���Ϣ*****************" << endl;

    cout << "�����ڴ�������" << (memPhys.ullTotalPhys * 1.0 / (1024 * 1024 * 1024)) << "GB" << endl;
    cout << "�����ڴ�ʹ���ʣ�" << memPhys.dwMemoryLoad << "%" << endl;
    cout << "�����ڴ�ʹ������" << ((memPhys.ullTotalPhys - memPhys.ullAvailPhys) * 1.0 / (1024 * 1024 * 1024)) << "GB" << endl;
    cout << "�����ڴ�������" << (memPhys.ullTotalVirtual * 1.0 / (1024 * 1024 * 1024)) << "GB" << endl;
    cout << "�����ڴ�ʹ������" << ((memPhys.ullTotalVirtual - memPhys.ullAvailVirtual) * 1.0 / (1024 * 1024)) << "MB" << endl;

    PERFORMANCE_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    pi.cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pi, sizeof(pi));

    cout << "ϵͳ��ǰ�ύ��ҳ��������" << pi.CommitTotal << "ҳ" << endl;
    cout << "ϵͳ�����ڴ�����" << pi.SystemCache << "ҳ" << endl;
    cout << "��ǰ��ҳ�ͷǷ�ҳ�ں˳��е��ڴ��ܺͣ�" << pi.KernelTotal << "ҳ" << endl;
    cout << "��ǰ��ҳ�ں˳��е��ڴ棺" << pi.KernelPaged << "ҳ" << endl;
    cout << "��ǰ�ڷǷ�ҳ�ں˳��е��ڴ棺" << pi.KernelNonpaged << "ҳ" << endl;
    cout << "�򿪾������" << pi.HandleCount << endl;
    cout << "��ǰ��������" << pi.ProcessCount << endl;
    cout << "��ǰ�߳�����" << pi.ThreadCount << endl;
    cout << endl;

}

void AllProcessID()
{
    cout << "*****************�鿴���н��̵������Ϣ*****************" << endl;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout << "CreateToolHelp32Snapshot ����ʧ��" << endl;
        return ;
    }
    PROCESSENTRY32 pe32;
    ZeroMemory(&pe32, sizeof(pe32));
    pe32.dwSize = sizeof(pe32);
    BOOL bMore = Process32First(hProcessSnap, &pe32);
    if (!bMore)
    {
        cout << "Process32First ����ʧ�ܣ�" << endl;
    }
    int num = 0;
    char head[10][10] = { "���","PID","����","���ȼ�" };
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
    cout << "*****************�鿴�ض����̵���Ϣ*****************" << endl;
    //��ý��̾��
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    //��ǰ���̵Ĺ�������С
    PROCESS_MEMORY_COUNTERS pmc;
    ZeroMemory(&pmc, sizeof(pmc));
    pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
    BOOL bt = GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
    if (bt)
        cout << "��ǰ���̵Ĺ�������С��" << pmc.WorkingSetSize * 1.0 / (1024 * 1024) << "MB" << endl;
    else
    {
        cout << "�鿴���̹�����ʧ�ܣ�" << endl;
        return;
    }
        

    MEMORY_BASIC_INFORMATION mbi;
    ZeroMemory(&mbi, sizeof(mbi));


    //hProcess = GetCurrentProcess();
    //��ý��̿��õĵ�ַ�ռ�
    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(si));
    GetSystemInfo(&si);
    LPVOID pBlock = si.lpMinimumApplicationAddress;

    char header[10][10] = { "��ķ�Χ","״̬","��������","�ļ�����","ģ����" };
    printf("%26s %15s %20s %10s %30s\n", header[0], header[1], header[2], header[3],header[4]);
    while (pBlock < si.lpMaximumApplicationAddress)
    {
        if (VirtualQueryEx(hProcess, pBlock, &mbi, sizeof(mbi)) == sizeof(mbi))
        {
            //�����Ľ�β�ͳ���
            LPVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;

            TCHAR szSize[MAX_PATH];
            StrFormatByteSize(mbi.RegionSize, szSize, MAX_PATH);

            printf("%08x-%08x (%8ls) ", pBlock, pEnd, szSize);
            /*cout.fill('0');
            cout << hex << setw(8) << (DWORD)pBlock
                << "-"
                << hex << setw(8) << (DWORD)pEnd
                << "("<<szSize<<")   ";*/

            //��ʾ���״̬
            switch (mbi.State)
            {
            case MEM_COMMIT:
                printf("%15s", "�ύ");
                //cout << "�ύ   ";
                break;
            case MEM_FREE:
                printf("%15s", "����");
                //cout << "����   ";
                break;
            case MEM_RESERVE:
                printf("%15s", "����");
                //cout << "����   ";
                break;
            }

            //��ʾ����

            if (mbi.Protect == 0 && mbi.State != MEM_FREE) {
                mbi.Protect = PAGE_READONLY;
            }
            showProtection(mbi.Protect);

            //��ʾ����
            switch (mbi.Type)
            {
            case MEM_IMAGE:
                printf("%10s ", "Image");
                //cout << ",Image";//ӳ���ļ�
                break;
            case MEM_MAPPED:
                printf("%10s ", "Mapped");
                //cout << ",Mapped";//�ڴ�ӳ���ļ�
                break;
            case MEM_PRIVATE:
                printf("%10s ", "Private");
                //cout << ",Private";//˽���ڴ���
                break;
            }

           
            //�����ִ�е�ӳ��
            TCHAR fileName[MAX_PATH];

            if (GetModuleFileName(
                (HMODULE)pBlock,
                fileName,
                MAX_PATH
            ) > 0)
            {
                //��ȥ·����ʾ
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
    cout << "*****************�ڴ������*****************" << endl;
    cout << "��Ŷ�Ӧ�Ĺ��ܣ�" << endl;
    cout << "1. ϵͳ�ڴ��ַ�ռ䲼��" << endl;
    cout << "2. ϵͳ�����ڴ���Ϣ" << endl;
    cout << "3. �鿴���н���" << endl;
    cout << "4. �鿴�ض����̵���Ϣ" << endl;
    cout << "0. �˳�" << endl;
    while (1)
    {
        cout << "��������ţ�";
        int n;
        cin >> n;
        switch (n)
        {
        case 1:
            showSystemInfo();
            cout << "��ʾ��ϣ�" << endl;
            break;
        case 2:
            showMemoryPhysInfo();
            cout << "��ʾ��ϣ�" << endl;
            break;
        case 3:
            AllProcessID();
            cout << "��ʾ��ϣ�" << endl;
            break;
        case 4:
            DWORD pid;
            cout << "��������̺�pid��";
            cin >> pid;
            showProcessInfo(pid);
            cout << "��ʾ��ϣ�" << endl;
            break;
        case 0:
            return 0;
        default:
            cout << "���벻���ϸ�ʽ" << endl;
            break;
        }

        
    }
    

    
    return 0;
}