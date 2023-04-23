#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h> 
#define N 5                                      // Количество поклонников
#define SEM_NAME "/valentines_day_semaphore"     // Имя семафора
#define SHM_NAME "/valentines_day_shared_memory" // Имя разделяемой памяти

typedef struct
{
    int valentine[N]; // Варианты романтического вечера для каждого поклонника
    int chosen_one;   // Индекс избранника
} SharedMemory;

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int main()
{
    int semaphore_id;            // Идентификатор семафора
    int shm_id;                  // Идентификатор разделяемой памяти
    SharedMemory *shared_memory; // Указатель на разделяемую память
    pid_t child_processes[N];    // Массив идентификаторов дочерних процессов

    // Создаем и инициализируем семафор
    semaphore_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0644);
    if (semaphore_id == -1)
    {
        perror("semget");
        exit(1);
    }

    union semun sem_arg;
    sem_arg.val = 1;
    if (semctl(semaphore_id, 0, SETVAL, sem_arg) == -1)
    {
        perror("semctl");
        exit(1);
    }

    // Создаем и открываем разделяемую память
    shm_id = shmget(IPC_PRIVATE, sizeof(SharedMemory), IPC_CREAT | 0644);
    if (shm_id == -1)
    {
        perror("shmget");
        exit(1);
    }

    // Отображаем разделяемую память в память процесса
    shared_memory = (SharedMemory *)shmat(shm_id, NULL, 0);
    if (shared_memory == (SharedMemory *)-1)
    {
        perror("shmat");
        exit(1);
    }

    // Инициализируем разделяемую память
    for (int i = 0; i < N; i++)
    {
        shared_memory->valentine[i] = rand() % 10;
    }
    shared_memory->chosen_one = -1;
    // Создаем дочерние процессы
    for (int i = 0; i < N; i++)
    {
        child_processes[i] = fork();
        if (child_processes[i] == -1)
        {
            perror("fork");
            exit(1);
        }
        else if (child_processes[i] == 0)
        {
            // Код выполняемый в дочернем процессе
            int valentine = shared_memory->valentine[i];
            printf("Студент %d отправляет валентинку %d\n", getpid(), valentine);
            sleep(1);

            // Операция P на семафоре
            struct sembuf sem_op;
            sem_op.sem_num = 0;
            sem_op.sem_op = -1;
            sem_op.sem_flg = 0;
            semop(semaphore_id, &sem_op, 1);

            // Проверяем, есть ли уже избранник
            if (shared_memory->chosen_one == -1)
            {
                // Если нет, то становимся избранником
                shared_memory->chosen_one = i;
                printf("Студент %d стал избранником и выбрал вариант %d\n", getpid(), valentine);

                // Операция V на семафоре
                sem_op.sem_op = 1;
                semop(semaphore_id, &sem_op, 1);
            }
            else
            {
                // Если есть, то просто освобождаем семафор
                printf("Студент %d не стал избранником и освобождает семафор\n", getpid());

                // Операция V на семафоре
                sem_op.sem_op = 1;
                semop(semaphore_id, &sem_op, 1);
            }

            exit(0);
        }
    }

    // Ожидаем завершения дочерних процессов
    for (int i = 0; i < N; i++)
    {
        waitpid(child_processes[i], NULL, 0);
    }

    // Удаляем семафор и разделяемую память
    semctl(semaphore_id, 0, IPC_RMID);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
