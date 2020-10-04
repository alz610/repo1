#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    size_t n = 128;         // длина строк в чанке
    size_t m = 100;         // длина чанка, кол-во строк
    size_t arrsize = 1000;  // кол-во элементов в массиве распарсенных чисел

    FILE *fp;
    float *arr;     // массив распарсенных чисел
    char *chunk0;   // чанк строк файла
    char *chunk1;   // следующий чанк строк файла

    // char *filename = "test"; // имя читаемого файла
    char *filename = argv[1]; // имя читаемого файла


    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open file.\n");
        return 1;
    }

    arr = (float *) malloc(arrsize * sizeof(float));
    chunk0 = (char *) malloc(n * m * sizeof(char));
    chunk1 = (char *) malloc(n * m * sizeof(char));

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



    clock_t begin = clock();


    // main 

    size_t nread0, nread1;

    float *p = arr;
    nread0 = getchunk(chunk0, &n, &m, fp);

    while (nread0 != 0)
    {
        #pragma omp parallel
        {
            #pragma omp sections
            {
                #pragma omp section
                {
                    parsechunk(&p, &n, &nread0, chunk0);
                }
                #pragma omp section
                {
                    nread1 = getchunk(chunk1, &n, &m, fp);
                }
            }
        }


        // swap

        {
        char *temp = chunk0;
        chunk0 = chunk1;
        chunk1 = temp;
        }

        {
        size_t temp = nread0;
        nread0 = nread1;
        nread1 = temp;
        }
    }


    double elapsed = (double)(clock() - begin) / CLOCKS_PER_SEC;


    printf("elapsed: %f ms\n", elapsed * 1000);

    if ((argc > 2) && (*argv[2] == 'a'))
    {
        /* float values were successfully read */
        for (float *p_ = arr; p_ < p; p_++)
            printf("arr[%d]=%f\n", (int) (p_ - arr), *p_);
    }


    fclose(fp);
    free(arr);
    free(chunk0);
    free(chunk1);

    return 0;
}