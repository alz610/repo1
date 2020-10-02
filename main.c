#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getchunk(char *chunk, size_t *n, size_t *m, FILE *fp)
{
    size_t i = 0, nread;

    while (!feof(fp) && (i < *m))
        fgets((chunk + *n * i++), *n, fp);

    nread = i;

    return nread;
}

int parsechunk(float *arr, size_t *n, char *chunk)
{
    size_t i = 0, j = 0, nread;

    while ((sscanf() != -1) && (i < *chksize))
    {
        // запись указателя на прочтенную строку в массив указателей
        chunk[i++] = line;

        // для автоматического выделения памяти для следующей читаемой строки у функции getline
        line = NULL;
        len = 0;
    }

    nread = j;

    return nread;
}

int main(int argc, char *argv[])
{
    size_t n = 128;           // длина строк в чанке
    size_t m = 10;            // длина чанка, кол-во строк
    size_t arrsize = 1000;    // кол-во элементов в массиве распарсенных вещественных чисел

    FILE *fp;
    float *arr;   // массив распарсенных вещественных чисел
    char *chunk0; // чанк строк файла
    char *chunk1; // следующий чанк строк файла
    int nread;

    char *filename = "test"; // имя читаемого файла
    // char *filename = argv[1]; // имя читаемого файла

    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open file.\n");
        return 1;
    }

    arr = (float *)malloc(arrsize * sizeof(float));
    chunk0 = (char *)malloc(n * m * sizeof(char));
    chunk1 = (char *)malloc(n * m * sizeof(char));

    if (arr == NULL)
    {
        printf("Unable to allocate arr");
        return 1;
    }
    if (chunk0 == NULL)
    {
        printf("Unable to allocate chunk0");
        return 1;
    }
    if (chunk1 == NULL)
    {
        printf("Unable to allocate chunk1");
        return 1;
    }

    nread = getchunk(chunk0, &n, &m, fp);

    parsechunk(arr, &n, &m, chunk0);


    // while (1)
    // {
    //     #pragma omp parallel sections
    //     {
    //         #pragma omp section
    //         {
    //             parsechunk(array, &chksize, chunk0);
    //         }
    //         #pragma omp section
    //         {
    //             nread = getchunk(chunk1, &chksize, stream);
    //         }
    //     }
    // }

    /* n-1 float values were successfully read */
    // for (size_t i = 0; i < n - 1; i++)
    //     printf("fval[%d]=%f\n", i, array[i]);

    printf("%s\n", chunk0);
    printf("%s\n", chunk0 + n * 1);

    fclose(fp);

    return 0;
}