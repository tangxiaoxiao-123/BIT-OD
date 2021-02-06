#include<Windows.h>
#include<stdio.h>
#include<string.h>

const int MAXLENGTH = 1024;

void changetime(WIN32_FIND_DATA wfd,const char* target)
{
    //获得文件句柄
    HANDLE hFile = CreateFile(
        target,       //目标文件名(路径)
        GENERIC_WRITE,//修改时间戳
        FILE_SHARE_READ, //请求读访问权的打开操作将会成功
        NULL,
        OPEN_EXISTING,//文件已经存在
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );
    SetFileTime(hFile,&wfd.ftCreationTime,&wfd.ftLastAccessTime,&wfd.ftLastWriteTime);
    CloseHandle(hFile);
    
}
void copyfile(const char* source,const char* target)
{
    HANDLE hsource = CreateFile(
        source,
        GENERIC_READ,//只读
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    HANDLE htarget = CreateFile(
        target,
        GENERIC_WRITE|GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,//如果文件不存在则创建
        FILE_ATTRIBUTE_ARCHIVE,
        NULL
    );
    DWORD fileSize = GetFileSize(hsource,NULL);
    DWORD readSize = 0;
    DWORD writeSize = 0;
    char *buffer = (char*)malloc(fileSize);
    //从源文件中读取文件到缓冲区中
    BOOL isok = ReadFile(
        hsource,    //用GENERIC_READ创建的文件句柄
		buffer,        //存放从文件读取数据的缓冲区的地址指针
		fileSize,//要从文件读的字节数
		&readSize,//存放实际读取字节数的变量地址
		NULL
        );
    if(!isok)
    {
        printf("read file error!\n");
        exit(0);
    } 
    isok = WriteFile(htarget,buffer,fileSize,&writeSize,NULL);
    if(!isok)
    {
        printf("write file error!\n");
        exit(0);
    }
    free(buffer);
    CloseHandle(hsource);
    CloseHandle(htarget);

}

void mycp(const char* source,const char* target)
{
    WIN32_FIND_DATA lpFindFileData;
    char sourcePath[MAXLENGTH] = "";
    char targetPath[MAXLENGTH] = "";

    //当前目录下的任意文件（文件名）
    strcat(sourcePath,source);
    strcat(sourcePath,"\\*.*");

    strcat(targetPath,target);
    strcat(targetPath,"\\");

    HANDLE hFind = FindFirstFile(sourcePath,&lpFindFileData);
    
    if(hFind != INVALID_HANDLE_VALUE)
    {
        //查找下一个文件
        bool flag = 1;
        
        while(flag)
        {
            //复制文件名字
            memset(sourcePath,0,sizeof(sourcePath));
            strcat(sourcePath,source);
            strcat(sourcePath,"\\");
            strcat(sourcePath,lpFindFileData.cFileName);

            memset(targetPath,0,sizeof(targetPath));
            strcat(targetPath,target);
            strcat(targetPath,"\\");
            strcat(targetPath,lpFindFileData.cFileName);

            

            //如果是文件夹的话，递归执行mycp
            if(lpFindFileData.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY)
            {
                if(strcmp(lpFindFileData.cFileName,".")!=0&&strcmp(lpFindFileData.cFileName,"..")!=0)
                {
                    //创建子目录
                    CreateDirectory(targetPath,NULL);
                    //复制子目录的内容
                    mycp(sourcePath,targetPath);
                    
                }
            }
            //如果是文件，那么直接复制就好
            else
            {
                copyfile(sourcePath,targetPath);
                
            }
            
            changetime(lpFindFileData,targetPath);
            flag = FindNextFile(hFind,&lpFindFileData);
        }
    }
    else
    {
        printf("hFind error!\n");
        exit(0);
    }
    
}


int main(int argc,char* argv[])
{
    if(argc!=3)
    {
        printf("parameter number must be 3!\n");
        printf("usage:mycp sourcePath targetPath\n");
        exit(0);
    }
    printf("copying from %s to %s\n",argv[1],argv[2]);
    WIN32_FIND_DATA lpFindFileDataSource;
    WIN32_FIND_DATA lpFindFileDataTarget;
    //查找指定文件路径的文件
    HANDLE hFindSource = FindFirstFile(argv[1],&lpFindFileDataSource);
    //源文件不存在
    if(hFindSource == INVALID_HANDLE_VALUE)
    {
        printf("source file is not existing!\n");
        exit(0);
    }
    
    //若目标文件夹不存在，则创建一个新的
    HANDLE hFindTarget = FindFirstFile(argv[2],&lpFindFileDataTarget);
    if(hFindTarget == INVALID_HANDLE_VALUE)
    {
        //创建一个新的文件夹
        CreateDirectory(argv[2],NULL);
        changetime(lpFindFileDataSource,argv[2]);
    }
    //关闭句柄
    CloseHandle(hFindSource);
    CloseHandle(hFindTarget);
    //复制文件
    mycp(argv[1],argv[2]);
    printf("copy success!\n");
    return 0;
}