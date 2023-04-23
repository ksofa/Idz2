#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#define N 5                                      // Количество поклонников
#define SEM_NAME "/valentines_day_semaphore"     // Имя семафора
#define SHM_NAME "/valentines_day_shared_memory" // Имя разделяемой памяти

typedef struct
{
    int valentine[N]; // Варианты романтического вечера для каждого поклонника
    int chosen_one;   // Индекс избранника
} SharedMemory;

int main()
{
    int shm_fd, sem_fd;
    sem_t *sem;
    SharedMemory *shared_memory;
    pid_t child_processes[N];

    sem_unlink(SEM_NAME);
    // Создаем и открываем именованный семафор
    sem_fd = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (sem_fd == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    sem_unlink(SHM_NAME);
    shm_unlink(SHM_NAME);
    // Создаем и открываем именованную разделяемую память
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0644);
    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    // Устанавливаем размер разделяемой памяти
    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1)
    {
        perror("ftruncate");
        exit(1);
    }

    // Отображаем разделяемую память в память процесса
    shared_memory = (SharedMemory *)mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    // Инициализируем разделяемую память
    for (int i = 0; i < N; i++)
    {
        shared_memory->valentine[i] = i + 2; // Варианты романтического вечера от 2 до 6
    }
    shared_memory->chosen_one = -1; // Индекс избранника изначально -1

    // Создаем дочерние процессы для поклонников
    for (int i = 0; i < N; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Код поклонника
            sem_t *sem = sem_open(SEM_NAME, 0); // Открываем семафор для чтения
            if (sem == SEM_FAILED)
            {
                perror("sem_open");
                exit(1);
            }

            // Код поклонника
            sem_wait(sem); // Ожидаем доступа к разделяемой памяти
            printf("Студент %d выбрал вариант %d\n", getpid(), shared_memory->valentine[i]);
            if (shared_memory->chosen_one == -1)
            {
                shared_memory->chosen_one = shared_memory->valentine[i]; // Выбираем первого поклонника как избранника
            }
            sem_post(sem); // Освобождаем доступа к разделяемой памяти
            exit(0);       // Завершаем процесс поклонника
        }
        else if (pid > 0)
        {
            child_processes[i] = pid; // Сохраняем идентификатор дочернего процесса
        }
        else
        {
            perror("fork");
            exit(1);
        }
    }

    // Ожидаем завершения всех дочерних процессов
    for (int i = 0; i < N; i++)
    {
        waitpid(child_processes[i], NULL, 0);
    }

    // Выводим результаты выборов
    printf("Итоги выборов:\n");
    printf("Красавица выбрала вариант %d\n", shared_memory->valentine[shared_memory->chosen_one]);

    // Закрываем и удаляем именованный семафор
    sem_close(sem);
    sem_unlink(SEM_NAME);

    // Отсоединяем разделяемую память и удаляем ее
    munmap(shared_memory, sizeof(SharedMemory));
    shm_unlink(SHM_NAME);

    return 0;
}
