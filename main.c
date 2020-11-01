#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "parse.h"


/*
Парсинг файла fp в массив чисел arr.

    arrsize -- длина массива распарсенных чисел (в числах)
    linesize -- длина строк чанка (в символах)
    chunksize -- длина чанка (в строках)
*/
int test0(size_t arrsize, size_t linesize, size_t chunksize)
{
    FILE *fp = fopen("data.txt", "r");              // читаемый текст
    float *arr = malloc(arrsize * sizeof(float));   // записываемый массив


    double st = omp_get_wtime();

    size_t nread = parsefile(arr, linesize, chunksize, fp);

    double total = omp_get_wtime() - st;


    printf("total time: %f ms\n", total * 1000);
    printf("total floats read: %zu\n", nread);

    /* float values were successfully read */
    // for (size_t i = 0; i < nread; i++)
    //     printf("arr[%zu]=%f\n", i, arr[i]);


    fclose(fp);
    free(arr);

    return 0;
}


/*
Парсинг data в массив чисел arr.
*/
int test1()
{
    size_t arrsize = 1000;  // кол-во элементов в массиве распарсенных чисел
    char* data = 1 + R"(
 4.6100037e-01 3.1107558e-02 4.5741891e-02 1.0877928e+01 1.1335820e+01
 1.9592941e+01 6.9261334e-03 1.9498415e-01 8.4225499e-01 3.5035370e-01
 2.4903279e+00 8.4508432e+00 9.4139468e+01 4.3782857e+01 3.4074724e+00
 6.7677397e-01 9.9332535e+01 4.3006734e-02 4.3427788e-01 3.6188581e-01)";

    size_t linesize = 128;         // длина строк в чанке
    size_t chunksize = 100;         // длина чанка, кол-во строк


    FILE *fp = fmemopen(data, strlen(data), "r");
    float *arr = malloc(arrsize * sizeof(float)); // массив распарсенных чисел


    double st = omp_get_wtime();

    size_t nread = parsefile(arr, linesize, chunksize, fp);

    double total = omp_get_wtime() - st;


    printf("total time: %f ms\n", total * 1000);
    printf("total floats read: %zu\n", nread);

    /* float values were successfully read */
    for (size_t i = 0; i < nread; i++)
        printf("arr[%zu]=%f\n", i, arr[i]);


    fclose(fp);
    free(arr);

    return 0;
}


int main(int argc, char *argv[])
{
    size_t cols = 5;                // длина строки текста (в числах float)
    size_t arrsize = 1000000;       // длина массива распарсенных чисел (в числах float)
    size_t chunksize_ = 10000;         // длина чанка (в числах float)

    size_t linesize = 128;                  // длина строк чанка (в символах char)
    size_t chunksize = chunksize_ / cols;   // длина чанка (в строках)


    printf("parallel program\n\n");
    test0(arrsize, linesize, chunksize);


    chunksize = arrsize / cols;

    printf("\n\nnonparallel program\n\n");
    test0(arrsize, linesize, chunksize);

    return 0;
}