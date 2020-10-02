#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getchunk(char **chunk, size_t *chksize, FILE *stream)
{
    // Read lines using POSIX function getline
    // This code won't work on Windows
    size_t i = 0, nread = 0;
    char *line = NULL;
    size_t len = 0;

    while ((getline(&line, &len, stream) != -1) && (i < *chksize))
    {
        // запись указателя на прочтенную строку в массив указателей
        chunk[i++] = line;

        // для автоматического выделения памяти следующей читаемой строки у функции getline
        line = NULL;
        len = 0;
    }

    nread = i;

    return nread;
}

// int parsechunk(float *array, size_t *chksize, char **chunk)
// {
//     char *line = NULL;
//     size_t len = 0;
//     size_t i = 0;

//     while ((sscanf() != -1) && (i < *chksize))
//     {
//         // запись указателя на прочтенную строку в массив указателей
//         chunk[i++] = line;

//         // для автоматического выделения памяти для следующей читаемой строки у функции getline
//         line = NULL;
//         len = 0;
//     }

//     return i;
// }

int main()
{
    char filename[] = "test";  // имя читаемого файла
    size_t chksize = 10;  // кол-во строк файла в чанке
    size_t arrsize = 1000;  // кол-во элементов в массиве распарсенных вещественных чисел

    FILE *stream;
    float *array;  // массив распарсенных вещественных чисел
    char *chunk0;  // чанк строк файла
    char *chunk1;  // следующий чанк строк файла
    int nread;


    stream = fopen(filename, "r");

    if (stream == NULL)
    {
        perror("failed to open file\n");
        exit(1);
    }


    array = (float *)malloc(arrsize * sizeof(float));
    chunk0 = (char **)malloc(chksize * sizeof(char*));
    chunk1 = (char **)malloc(chksize * sizeof(char*));

    if (array == NULL)
    {
        perror("Unable to allocate array");
        exit(1);
    }
    if (chunk0 == NULL)
    {
        perror("Unable to allocate chunk0");
        exit(1);
    }
    if (chunk1 == NULL)
    {
        perror("Unable to allocate chunk1");
        exit(1);
    }


    nread = getchunk(chunk0, &chksize, stream);

    // while (nread == chksize)
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

    fclose(stream);

    exit(0);
}