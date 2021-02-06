#include <Windows.h>
#include <stdio.h>
#include<iostream>
using namespace std;

//�õ���ǰʱ��
long int GetTime()
{
    SYSTEMTIME time;
    ZeroMemory(&time, sizeof(time));
    GetSystemTime(&time);
    //���ʱ��
    cout<<time.wYear<<"/"<<time.wMonth<<"/"<<time.wDay<<", "<<time.wHour+8
    <<"h:"<<time.wMinute<<"m:"<<time.wSecond<<"s:"<<time.wMilliseconds<<"ms"<<endl;
    //�õ��Ժ���Ϊ��λ��ʱ��������������
    long int resTime = time.wMinute*60*1000+time.wSecond*1000+time.wMilliseconds;
    return resTime;
}

    
void Parent(char *clonename)
{

    //��ʽ���ӽ��̵�������
    TCHAR szCmdLine[MAX_PATH];
    sprintf(szCmdLine, "%s", clonename);

    //�ӽ���������Ϣ
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    //�ӽ��̵��й���Ϣ
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    //�ӽ��̿�ʼʱ��
    cout<<"Start time: ";
    long int startTime = GetTime();

    //�����ӽ���
    BOOL bCreateOK = CreateProcess(
        NULL,//��ִ�е�Ӧ�ó������ƣ��������ΪNULL��
        szCmdLine,//Ҫִ�е�������
        NULL,//ȱʡ�Ľ��̰�ȫ��
        NULL,//ȱʡ���̰߳�ȫ��
        FALSE,//���̳о��
        CREATE_NEW_CONSOLE,//�����µĿ���̨
        NULL,//ʹ�ø����̵Ļ�����
        NULL,//ʹ�ø����̵Ĺ���Ŀ¼
        &si,//������Ϣ
        &pi);//�����½��̺��̵߳Ľṹ��Ϣ
    if (bCreateOK)
    {
        printf("Create ChildProcess Succuss!\n");
        WaitForSingleObject(pi.hProcess, INFINITE);

        //�ӽ��̽���ʱ��
        cout<<"End time: ";
        long int endTime = GetTime();
        printf("the interval time:%lld\n", endTime - startTime);

        //�رվ������ֹ��Դ�˷�
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

