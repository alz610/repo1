#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "parse.h"
#include "test0.h"


/*
Программа запускает парсинг данных из `data.txt` в массив чисел `arr` и замеряет время исполнения.
*/
int main()
{
    // DEBUG = 1;      // вывод информации работы: измерение времени исполнения
    VERBOSE = 1;    // вывод отладочной информации
    // TEST = 1;       // воспроизведение ассертов


    size_t cols = 5;                // длина строки текста (в числах float)
    size_t arrsize = 1000000;       // длина массива распарсенных чисел (в числах float)
    size_t chunksize_ = 10000;         // длина чанка (в числах float)

    size_t linesize = 0x100;                  // длина строк чанка (в символах char)
    size_t chunksize = chunksize_ / cols;   // длина чанка (в строках)


    FILE *fp = fopen("data.txt", "r");              // читаемый файл
    float *arr = malloc(arrsize * sizeof(float));   // записываемый массив чисел


    double st = omp_get_wtime();

    size_t nread = parsefile(arr, linesize, chunksize, fp);

    double total = omp_get_wtime() - st;


    if (VERBOSE)
    {
        fprintf(stderr, "total time: %f ms\n", total * 1000);
        // fprintf(stderr, "total floats read: %zu\n", nread);
    }


    fclose(fp);
    free(arr);

    fflush(stderr);

    return nread;
}
