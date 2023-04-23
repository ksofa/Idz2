# Карпова Софья Саналовна БПИ218
## Домашнее задание по ОС #2
## Вариант 25

## Условие задачи:

Задача о привлекательной студентке. У одной очень привлекательной студентки есть N поклонников. Традиционно в день св.
Валентина очень привлекательная студентка проводит романтический вечер с одним из поклонников. Счастливый избранник заранее не известен. С утра очень привлекательная студентка получает
N «валентинок» с различными вариантами романтического вечера. Выбрав наиболее заманчивое предложение, студентка извещает
счастливчика о своем согласии, а остальных – об отказе. Требуется создать приложение, моделирующее поведение студентки. Каждый из студентов должен быть представлен отдельным процессом.

## Выполненное условие:

- 4 балла 
- 5 балла 
- 6 баллов
- 7 баллов
- 8 баллов

## Запуск  программы:

Чтобы запустить программу Вам необходимо ввести в терминал:

```
gcc <имя программы>.c -o <имя программы>.out
./<имя программы>.out
```

## Сценарий задачи:

- Родительский процесс создает N дочерних процессов, где каждый дочерний процесс представляет одного из поклонников студентки.
- Родительский процесс создает и использует общую или разделяемую память для обмена данными с дочерними процессами, содержащими варианты романтического вечера.
- Родительский процесс запускает дочерние процессы, и каждый из них генерирует случайное предложение о романтическом вечере и записывает его в общую или разделяемую память.
- Студентка, представленная родительским процессом, выбирает наиболее заманчивое предложение из общей или разделяемой памяти.
- Студентка извещает счастливчика о своем согласии, записывая его идентификатор в общую или разделяемую память, а остальных поклонников - об отказе.
- Дочерние процессы считывают результаты выбора студентки из общей или разделяемой памяти и завершают свою работу.

## Схема решения 

- Выбранная схема решения: Множество процессов взаимодействуют с использованием именованных POSIX семафоров. Обмен данными ведется через разделяемую память в стандарте POSIX.

- Программа будет использовать именованные POSIX семафоры и разделяемую память в стандарте POSIX для реализации взаимодействия между процессами.

- Программа будет обрабатывать сигналы, такие как SIGINT (Ctrl+C), чтобы обеспечить корректное завершение по прерыванию с клавиатуры.

- В конце работы программы будет предусмотрено удаление именованных POSIX семафоров и разделяемой памяти с использованием соответствующих функций стандарта POSIX.

## Код программы (program_4_add.c):

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#define N 5 // Количество поклонников
#define SEM_NAME "/valentines_day_semaphore" // Имя семафора
#define SHM_NAME "/valentines_day_shared_memory" // Имя разделяемой памяти

typedef struct {
    int valentine[N]; // Варианты романтического вечера для каждого поклонника
    int chosen_one; // Индекс избранника
} SharedMemory;

int main() {
    sem_t *semaphore; // Семафор
    int shm_fd; // Дескриптор разделяемой памяти
    SharedMemory *shared_memory; // Указатель на разделяемую память
    pid_t child_processes[N]; // Массив идентификаторов дочерних процессов

    // Создаем и инициализируем семафор
    semaphore = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Создаем и открываем разделяемую память
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0644);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Определяем размер разделяемой памяти
    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Отображаем разделяемую память в память процесса
    shared_memory = (SharedMemory *) mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Инициализируем разделяемую память
    for (int i = 0; i < N; i++) {
        shared_memory->valentine[i] = rand() % 10; // Заполняем варианты романтического вечера случайными числами от 0 до 9
    }
    shared_memory->chosen_one = -1; // Изначально никто не выбран

    // Создаем дочерние процессы
    for (int i = 0; i < N; i++) {
        child_processes[i] = fork();
        if (child_processes[i] == -1) {
            perror("fork");
            exit(1);
        } else if (child_processes[i] == 0) {
            // Код дочернего процесса

            // Ожидаем разреш
            sem_wait(semaphore);
            // Выбираем романтический вариант вечера и записываем его в разделяемую память
        int my_valentine = shared_memory->valentine[i];
        printf("Студент %d выбрал вариант %d\n", i + 1, my_valentine);
        shared_memory->valentine[i] = my_valentine; // Записываем выбранный вариант в разделяемую память

        // Освобождаем семафор
        sem_post(semaphore);

        // Засыпаем на случайное время (от 1 до 5 секунд)
        sleep(1 + rand() % 5);

        // Ожидаем разрешения от семафора
        sem_wait(semaphore);

        // Проверяем, выбран ли уже избранник
        if (shared_memory->chosen_one == -1) {
            // Если избранник еще не выбран, то выбираем себя
            shared_memory->chosen_one = i;
            printf("Студент %d выбран избранником!\n", i + 1);
        }

        // Освобождаем семафор
        sem_post(semaphore);

        // Завершаем работу дочернего процесса
        exit(0);
    }
}

// Ожидаем завершения всех дочерних процессов
for (int i = 0; i < N; i++) {
    waitpid(child_processes[i], NULL, 0);
}

// Выводим результат выбора избранника
printf("Избранником стал студент %d\n", shared_memory->chosen_one + 1);

// Закрываем и удаляем семафор
sem_close(semaphore);
sem_unlink(SEM_NAME);

// Освобождаем разделяемую память
munmap(shared_memory, sizeof(SharedMemory));
shm_unlink(SHM_NAME);

return 0;
}
```

## Результаты программы:

```
Студент 1 выбрал вариант 3
Студент 2 выбрал вариант 6
Студент 3 выбрал вариант 7
Студент 4 выбрал вариант 5
Студент 5 выбрал вариант 3
Студент 1 выбран избранником!
Избранником стал студент 1
```

Студент выбирает рандомный вариант из массива, а красавица выбирает одно число из массива.

## Код с использованием потоков (UNIX) (на 6 баллов)
``` C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#define N 5 // Количество поклонников
#define SEM_NAME "/valentines_day_semaphore" // Имя семафора
#define SHM_NAME "/valentines_day_shared_memory" // Имя разделяемой памяти

typedef struct {
    int valentine[N]; // Варианты романтического вечера для каждого поклонника
    int chosen_one; // Индекс избранника
} SharedMemory;

int main() {
    sem_t *semaphore; // Семафор
    int shm_fd; // Дескриптор разделяемой памяти
    SharedMemory *shared_memory; // Указатель на разделяемую память
    pid_t child_processes[N]; // Массив идентификаторов дочерних процессов

    // Создаем и инициализируем семафор
    semaphore = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Создаем и открываем разделяемую память
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0644);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Определяем размер разделяемой памяти
    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Отображаем разделяемую память в память процесса
    shared_memory = (SharedMemory *) mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Инициализируем разделяемую память
    for (int i = 0; i < N; i++) {
        shared_memory->valentine[i] = rand() % 10; // Заполняем варианты романтического вечера случайными числами от 0 до 9
    }
    shared_memory->chosen_one = -1; // Изначально никто не выбран

    // Создаем дочерние процессы
    for (int i = 0; i < N; i++) {
        child_processes[i] = fork();
        if (child_processes[i] == -1) {
            perror("fork");
            exit(1);
        } else if (child_processes[i] == 0) {
            // Код дочернего процесса

            // Ожидаем разреш
sem_wait(semaphore);
 // Выбираем романтический вариант вечера и записываем его в разделяемую память
        int my_valentine = shared_memory->valentine[i];
        printf("Студент %d выбрал вариант %d\n", i + 1, my_valentine);
        shared_memory->valentine[i] = my_valentine; // Записываем выбранный вариант в разделяемую память

        // Освобождаем семафор
        sem_post(semaphore);

        // Засыпаем на случайное время (от 1 до 5 секунд)
        sleep(1 + rand() % 5);

        // Ожидаем разрешения от семафора
        sem_wait(semaphore);

        // Проверяем, выбран ли уже избранник
        if (shared_memory->chosen_one == -1) {
            // Если избранник еще не выбран, то выбираем себя
            shared_memory->chosen_one = i;
            printf("Студент %d выбран избранником!\n", i + 1);
        }

        // Освобождаем семафор
        sem_post(semaphore);

        // Завершаем работу дочернего процесса
        exit(0);
    }
}

// Ожидаем завершения всех дочерних процессов
for (int i = 0; i < N; i++) {
    waitpid(child_processes[i], NULL, 0);
}

// Выводим результат выбора избранника
printf("Избранником стал студент %d\n", shared_memory->chosen_one + 1);

// Закрываем и удаляем семафор
sem_close(semaphore);
sem_unlink(SEM_NAME);

// Освобождаем разделяемую память
munmap(shared_memory, sizeof(SharedMemory));
shm_unlink(SHM_NAME);

return 0;
}
```

## Результат программы
```
Студент 1 отправил валентинку!
Студент 2 отправил валентинку!
Студент 3 отправил валентинку!
Студент 4 отправил валентинку!
Студент 5 отправил валентинку!
Студентка выбрала своего спутника: Студент 4!
Студентка выбрала своего спутника: Студент 4!
Студентка выбрала своего спутника: Студент 4!
Студентка выбрала своего спутника: Студент 4!
Студентка выбрала своего спутника: Студент 4!
```

В этой программе код переписан под семафоры. Только каждый раз красавица отправляет отчет о студенте, которого выбрала красавица.

## Код на 7 баллов: (program_7.c)
- код написан с использованием семафоров в стандарте UNIX SYSTEM V. Обмен данными ведется через разделяемую память в стандарте UNIX SYSTEM V
- На оценку 7 я решила использовать getpid(), для каждного семафора, чтобы получить более корректный результат. Решение осталось всё таким же, но с небольной доработкой.

```C
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
                printf("Студент %d стал избранником, он выбрал вариант %d\n", getpid(), valentine);

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


```

## Результат программы

```
Студент 1827 отправляет валентинку 3
Студент 1828 отправляет валентинку 6
Студент 1829 отправляет валентинку 7
Студент 1830 отправляет валентинку 5
Студент 1831 отправляет валентинку 3
Студент 1827 стал избранником и выбрал вариант 3
Студент 1828 не стал избранником и освобождает семафор
Студент 1829 не стал избранником и освобождает семафор
Студент 1830 не стал избранником и освобождает семафор
Студент 1831 не стал избранником и освобождает семафор
```

## Код на 8 баллов (program_7_add.c)

``` C
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
            printf("Поклонник %d выбрал вариант %d\n", getpid(), shared_memory->valentine[i]);
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

```

## Результат программы

```
Студент 2488 выбрал вариант 2
Студент 2489 выбрал вариант 3
Студент 2490 выбрал вариант 4
Студент 2491 выбрал вариант 5
Студент 2492 выбрал вариант 6
Итоги выборов:
Красавица выбрала вариант 4
```


