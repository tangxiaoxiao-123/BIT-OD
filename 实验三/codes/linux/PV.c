#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>

#define SEM_ID 250 //信号量的关键字
#define SHMKEY 80  //共享内存区的关键字
char dic[3] = {'T', 'X', 'J'};
//缓冲区
struct buf
{
    char text[4];
    int write_op;
    int read_op;
};

union senum
{
    int val;
    struct semid_ds *buf;
    ushort *array;
};

void P(int sem_id, int index)
{
    struct sembuf sem_op;
    sem_op.sem_num = index;
    sem_op.sem_flg = 0;
    sem_op.sem_op = -1;
    semop(sem_id, &sem_op, 1);
}

void V(int sem_id, int index)
{
    struct sembuf sem_op;
    sem_op.sem_num = index;
    sem_op.sem_flg = 0;
    sem_op.sem_op = 1;
    semop(sem_id, &sem_op, 1);
}

void Producer()
{
    //printf("I am a Producer.\n");

    //打开信号量集合
    int sem_id = semget(SEM_ID, 3, IPC_CREAT | 0600);
    if (sem_id == -1)
    {
        perror("producer semget error");
        exit(0);
    }

    for (int i = 0; i < 4; i++)
    {
        sleep(rand() % 3 + 1);
        int shm_id = shmget(SHMKEY, sizeof(buf), 0777 | IPC_CREAT);
        if (shm_id == -1)
        {
            perror("producer shmget error");
            exit(0);
        }
        struct buf *buffer = (struct buf *)shmat(shm_id, 0, 0);
        P(sem_id, 2); //P(empty)
        P(sem_id, 0); //P(mutex)
        int rand_num = rand() % 3;
        buffer->text[buffer->write_op] = dic[rand_num];
        buffer->write_op = (buffer->write_op + 1) % 4;
        printf("the producer: push %c\n", dic[rand_num]);
        printf("the buffer-text:");
        for (int i = 0; i < 4; i++)
        {
            printf("%c", buffer->text[i]);
        }
        printf("\n");
        shmdt(buffer);
        V(sem_id, 0); //V(mutex)
        V(sem_id, 1); //V(full)
    }
}
void Consumer()
{
    //printf("I am a Consumer.\n");
    int sem_id = semget(SEM_ID, 3, IPC_CREAT | 0600);
    if (sem_id == -1)
    {
        perror("consumer semget error");
        exit(0);
    }

    for (int i = 0; i < 3; i++)
    {
        sleep(rand() % 3 + 1);
        int shm_id = shmget(SHMKEY, sizeof(buf), 0777 | IPC_CREAT);
        if (shm_id == -1)
        {
            perror("consumer shmget error");
            exit(0);
        }
        struct buf *buffer = (struct buf *)shmat(shm_id, 0, 0);
        P(sem_id, 1); //P(full)
        P(sem_id, 0); //P(mutex)
        char c = buffer->text[buffer->read_op];
        buffer->text[buffer->read_op] = '-';
        buffer->read_op = (buffer->read_op + 1) % 4;
        printf("the consumer: pop %c\n", c);
        printf("the buffer-text:");
        for (int i = 0; i < 4; i++)
        {
            printf("%c", buffer->text[i]);
        }
        printf("\n");
        shmdt(buffer);
        V(sem_id, 0); //V(mutex)
        V(sem_id, 2); //V(empty)
    }
}
int main()
{
    srand((unsigned)time(NULL));
    //创建一个信号量集合，有三个信号量分别是mutex,full,empty
    int sem_id = semget(SEM_ID, 3, IPC_CREAT | 0600);
    if (sem_id == -1)
    {
        perror("semget error");
        exit(0);
    }
    //对信号量进行初始化
    //mutex=1,full=0,empty=4
    union senum sem_init;
    ushort val[3] = {1, 0, 4};
    sem_init.array = val;
    int rc = semctl(sem_id, 0, SETALL, sem_init);
    if (rc == -1)
    {
        perror("semctl init error");
        exit(0);
    }
    //创建共享内存区
    int shm_id = shmget(SHMKEY, sizeof(buf), 0777 | IPC_CREAT);
    if (shm_id == -1)
    {
        perror("shmget error");
        exit(0);
    }

    //内存区映射到进程地址空间
    struct buf *buffer = (struct buf *)shmat(shm_id, 0, 0);
    //对buffer初始化
    buffer->write_op = 0;
    buffer->read_op = 0;
    for (int i = 0; i < 4; i++)
    {
        buffer->text[i] = '-';
    }
    //解除映射关系
    shmdt(buffer);

    //生产者
    for (int i = 0; i < 3; i++)
    {

        int pid = fork();

        if (pid == -1)
        {
            printf("创建失败！\n");
        }
        else if (pid == 0)
        {
            Producer();
            exit(0);
        }
    }

    //消费者
    for (int i = 0; i < 4; i++)
    {

        int pid = fork();

        if (pid == -1)
        {
            printf("创建失败！\n");
        }
        else if (pid == 0)
        {
            Consumer();
            exit(0);
        }
    }

    //等待7个子进程运行完
    for (int i = 0; i < 7; i++)
    {
        wait(0);
    }

    //对信号量和共享内存区进行删除
    semctl(sem_id, 0, IPC_RMID);
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}