#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "parse.h"
#include "main.h"


/*
Программа запускает парсинг файла `test.txt` в массив чисел `arr`,
включает отладочную информацию и ассерты, и замеряет время исполнения.
*/
int test0()
{
    DEBUG_LVL = 2;  // вывод отладочной информации
    TEST = 1;       // воспроизведение ассертов


    size_t cols = 5;                // длина строки текста (в числах float)
    size_t arrsize = 1000000;       // длина массива распарсенных чисел (в числах float)
    size_t chunksize_ = 10000;         // длина чанка (в числах float)

    size_t linesize = 0x100;                  // длина строк чанка (в символах char)
    size_t chunksize = chunksize_ / cols;   // длина чанка (в строках)


    FILE *fp = fopen("test.txt", "r");              // читаемый файл
    float *arr = malloc(arrsize * sizeof(float));   // записываемый массив чисел


    double st = omp_get_wtime();

    size_t nread = parsefile(arr, linesize, chunksize, fp);

    double total = omp_get_wtime() - st;


    if (DEBUG_LVL >= 1)
    {
        fprintf(stderr, "total time: %f ms\n", total * 1000);
        // fprintf(stderr, "total floats read: %zu\n", nread);
    }


    fclose(fp);
    free(arr);

    fflush(stderr);

    return nread;
}


/*
Программа запускает парсинг данных из строки `data` в массив чисел `arr`,
включает отладочную информацию и ассерты, и замеряет время исполнения.
*/
int test1()
{
    DEBUG_LVL = 2;  // вывод отладочной информации
    TEST = 1;       // воспроизведение ассертов


    size_t arrsize = 1000;  // кол-во элементов в массиве распарсенных чисел

    // плюс единица для исключения перехода на новую строку в начале строки data
    char* data = 1 + R"(
3.835915e-03 9.119928e+02 3.097778e-03 3.313361e+01 7.619404e-03
6.642741e-01 1.029128e+01 1.287661e+01 4.820329e-01 2.915097e-03
3.267618e-02 1.643332e+01 3.252714e-01 5.411628e-01 4.373466e-03
1.811544e+02 8.194344e+01 6.617470e+00 5.088499e-03 2.582988e+01
8.127293e+00 2.231672e+01 3.846160e-03 1.681731e-01 5.090333e+02
3.924403e-03 6.899752e-03 6.298465e+00 9.636068e+01 1.204787e-02
1.014477e-02 1.596934e-01 1.139246e-01 9.679427e+01 1.287466e-03
4.302530e-01 6.956054e-02 6.436195e-02 1.294139e+00 4.064663e-01
2.255975e+02 3.934291e+00 4.371512e+01 9.389749e+02 1.068752e+01
7.655255e-03 8.956400e+00 1.111476e-03 1.889590e-02 1.177082e-01
4.744304e+00 1.451626e+01 9.884526e+00 4.869323e+00 1.155277e+01
2.909246e+02 4.630600e-02 3.884705e-03 8.270984e-02 1.247382e-01
2.617648e+00 3.672331e+01 1.377019e+01 1.991189e-02 7.084127e-01
7.203352e+01 6.053259e+01 6.176124e-01 4.801809e-03 1.437850e+00
2.680943e+00 2.205157e-03 1.676864e+02 5.230805e+00 1.951069e+01
1.684111e+01 1.982992e-03 4.083021e+02 1.623807e+01 1.892871e+01
)";

    size_t linesize = 256;              // длина строк в чанке
    size_t chunksize_lines = 3 * 2;     // длина чанка (в строках)


    fprintf(stderr, "-------------------------\n");
    fprintf(stderr, "run\n\n");


    FILE *fp = fmemopen(data, strlen(data), "r");
    float *arr = malloc(arrsize * sizeof(float));   // записываемый массив чисел


    double st = omp_get_wtime();

    size_t nread = parsefile(arr, linesize, chunksize_lines, fp);

    double total = omp_get_wtime() - st;


    if (DEBUG_LVL >= 1)
    {
        fprintf(stderr, "total time: %f ms\n", total * 1000);
    }


    fclose(fp);
    free(arr);
    
    fflush(stderr);

    return nread;
}


int main(int argc, char const *argv[])
{
    test0();
    // test1();

    return 0;
}
