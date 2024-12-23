#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <cmath>
#include <locale.h>
#include <vector>
#include <algorithm>
#include <stdbool.h>
#include <iostream>


// Пользовательская функция для операции AND
void bool_and(void* invec, void* inoutvec, int* len, MPI_Datatype* datatype) {
    int i;
    if (*datatype != MPI_C_BOOL) {
        if (*datatype != MPI_INT) {
            printf("Ошибка: bool_and поддерживает только типы данных MPI_C_BOOL или MPI_INT.\n");

            return;
        }
        for (i = 0; i < *len; i++) {
            ((bool*)inoutvec)[i] = ((bool*)inoutvec)[i] && ((int*)invec)[i];
        }
        return;
    }
    for (i = 0; i < *len; i++) {
        ((bool*)inoutvec)[i] = ((bool*)inoutvec)[i] && ((bool*)invec)[i];
    }
}


void Eratosthenes(bool* Koskino, int n)
{
    // Алгоритм решета Эратосфена.
    for (int p = 2; p * p <= n; p++) {
        // Если p - простое число, то все его кратные, начиная с p*p, не являются простыми.
        if (Koskino[p]) {
            for (int i = p * p; i <= n; i += p) {
                Koskino[i] = false;
            }
        }
    }
}

void EratosthenesRange(bool* Koskino, int border, int start, int end)
{
    int trueStart;
    for (int p = 2; p <= border; p++) {
        if (Koskino[p]) {
            trueStart = start - (start % p);
            if (trueStart < start) { trueStart += p; }
            for (int i = trueStart; i <= end; i += p) {
                Koskino[i] = false;
                //printf("p - %d, i - %d, border - %d, start - %d, ent - %d\n", p, i, border, start, end);
            }
        }
    }
}

int main(int argc, char* argv[]) {

    setlocale(LC_ALL, "Russian");

    int n = pow(10, 8);

    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double t1, t2;
    int border = ceil(sqrt(n));
    int part = ceil((n - border) / size); // Размер диапазона, который будет вычислять один процесс
    int partForRank = part * rank + border; // Начало этого диапазона
    if (rank == size - 1) {
        part = (n - border) - (part * (size - 1));
    }


    bool* Koskino = (bool*)malloc((n + 1) * sizeof(bool));
    bool* result = (bool*)malloc((n + 1) * sizeof(bool));


    printf("Размер массива: %d\n", n);

    // Инициализируем все числа как простые.
    for (int i = 2; i <= n; i++) {
        Koskino[i] = true;
    }
    Koskino[0] = false;
    Koskino[1] = false;

    if (rank == 0) { printf("Диапазон базовых чисел [2, %d]\n", border); }
    printf("Процесс %d имеет диапазон [%d, %d]\n", rank, partForRank, partForRank + part);

    t1 = MPI_Wtime();
    Eratosthenes(Koskino, border); // Нахождение базовых простых чисел
    EratosthenesRange(Koskino, border, partForRank, partForRank + part); // Нахождение простых чисел

    MPI_Op bool_and_op;
    MPI_Op_create(bool_and, true, &bool_and_op);
    MPI_Reduce(Koskino, result, n + 1, MPI_C_BOOL, bool_and_op, 0, MPI_COMM_WORLD);

    MPI_Op_free(&bool_and_op);
    t2 = MPI_Wtime();

    /*if (rank == 0) {
        // Выводим все простые числа.
        printf("Простые числа до %d:\n", n);
        for (int i = 0; i <= n; i++) {
            if (result[i]) {
                printf("%4d ", i);
            }
        }
        printf("\n");
    }*/
    if (rank == 0) { printf("Время нахождения простых чисел: %f сек.\n", t2 - t1); }

    free(Koskino);
    free(result);

    MPI_Finalize();

    return 0;
}
