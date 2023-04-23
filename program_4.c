#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define N 10 // Количество студентов

int chosen_one = -1; // Индекс выбранного студента
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Мьютекс для синхронизации доступа к переменной chosen_one
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; // Условная переменная для ожидания выбора студентки

void* student(void* arg) {
    int id = *((int*) arg);
    printf("Студент %d отправил валентинку!\n", id);

    pthread_mutex_lock(&mutex); // Захватываем мьютекс

    while (chosen_one == -1) {
        pthread_cond_wait(&cond, &mutex); // Ожидаем выбора студентки
    }

    printf("Студентка выбрала своего спутника: Студент %d!\n", chosen_one + 1);

    pthread_mutex_unlock(&mutex); // Освобождаем мьютекс

    pthread_exit(NULL);
}

int main() {
    pthread_t students[N];
    int ids[N];

    // Создание дочерних потоков для каждого студента
    for (int i = 0; i < N; i++) {
        ids[i] = i + 1;
        pthread_create(&students[i], NULL, student, &ids[i]);
    }

    // Ожидание выбора студентки
    sleep(1); // Пауза для обеспечения случайного выбора студентки
    pthread_mutex_lock(&mutex); // Захватываем мьютекс
    chosen_one = rand() % N; // Выбираем случайного студента
    pthread_cond_broadcast(&cond); // Сигнализируем всем студентам о выборе
    pthread_mutex_unlock(&mutex); // Освобождаем мьютекс

    // Ожидание завершения работы всех потоков
    for (int i = 0; i < N; i++) {
        pthread_join(students[i], NULL);
    }

    pthread_mutex_destroy(&mutex); // Уничтожение мьютекса
    pthread_cond_destroy(&cond); // Уничтожение условной переменной

    return 0;
}
