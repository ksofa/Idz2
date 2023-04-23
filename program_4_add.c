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