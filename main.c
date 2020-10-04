#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getchunk(char *chunk, size_t *n, size_t *m, FILE *fp)
{
    size_t i, nread;

    for (i = 0; i < *m; i++)
    {
        fgets((chunk + *n * i), *n, fp);
        if (feof(fp)) break;
    }
    nread = i;

    return nread;
}

int parsechunk(float **p, size_t *n, size_t *m, char *chunk)
{
    size_t j = 0, nread;
    char *k;
    float parsed_num;

    for (size_t i = 0; i < *m; i++)
    {
        k = strtok(chunk + *n * i, " ");
        do {
            if (parsed_num = atof(k))
                *((*p)++) = parsed_num;
        } while (k = strtok(NULL, " "));
    }

    return 0;
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
    size_t nread;

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

    float *p = arr;

    parsechunk(&p, &n, &nread, chunk0);

    // while (1)
    // {
    //     #pragma omp parallel sections
    //     {
    //         #pragma omp section
    //         {
    //             parsechunk(&p, &n, &nread, chunk0);
    //         }
    //         #pragma omp section
    //         {
    //             nread = getchunk(chunk1, &n, &m, fp);
    //         }
    //     }
    // }

    /* float values were successfully read */
    for (float *p_ = arr; p_ < p; p_++)
        printf("arr[%d]=%f\n", (int) (p_ - arr), *p_);

    fclose(fp);

    return 0;
}