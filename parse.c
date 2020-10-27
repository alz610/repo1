#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>


/*
Чтение чанка файла fp в чанк chunk.

Ввод:
chunk -- записываемый чанк
    n -- длина строк чанка
    m -- длина чанка (в строках)
   fp -- читаемый файл

Вывод:
nread -- кол-во прочитанных строк
*/
size_t getchunk(char *chunk, size_t n, size_t m, FILE *fp)
{
    size_t i;

    for (i = 0; i < m; i++)
    {
        if (feof(fp)) break;
        fgets((chunk + n * i), n, fp);
    }

    size_t nread = i;

    return nread;
}

/*
Функция парсит чанка chunk и записывает распарсенные числа
в позицию *p в записываемом массиве.
Обновляет позицию *p.

Ввод:
   *p -- позиция в записываемом массиве чисел
    n -- длина строк чанка
    m -- длина чанка (в строках)
chunk -- читаемый чанк

Вывод:
    нет
*/
int parsechunk(float **p, size_t n, size_t m, char *chunk)
{
    char *k;
    float parsed_num;

    for (size_t i = 0; i < m; i++)
    {
        if (k = strtok(chunk + n * i, " "))
        {
            do {
                if (parsed_num = atof(k))
                    *((*p)++) = parsed_num;
            } while (k = strtok(NULL, " "));
        }
    }

    return 0;
}

/*
Парсинг файла fp в массив чисел arr.

Ввод:
  arr -- записываемый массив
    n -- длина строк чанка
    m -- длина чанка (в строках)
   fp -- читаемый файл

Вывод:
nread -- кол-во прочитанных чисел
*/
size_t parsefile(float *arr, size_t n, size_t m, FILE *fp)
{
    double t_read = 0, t_parse = 0;
    size_t nread0 = 0, nread1 = 0;
    // double t_total = 0;

    char *chunk0 = malloc(n * m * sizeof(char));   // предыдущий чанк
    char *chunk1 = malloc(n * m * sizeof(char));   // следующий чанк

    float *p = arr;  // позиция в записываемом массиве
  

    {
        double st = omp_get_wtime();
        
        nread0 = getchunk(chunk0, n, m, fp);
        
        double end = omp_get_wtime();
        t_read += end - st;
        // t_total += end - st;
    }

    while (nread0 != 0)
    {
        double st = omp_get_wtime();

        #pragma omp parallel
        {
            #pragma omp sections
            {
                /*
                Поток парсит предыдущий чанк и обновляет позицию p в массиве arr
                */
                #pragma omp section
                {
                    double st = omp_get_wtime();

                    parsechunk(&p, n, nread0, chunk0);

                    t_parse += omp_get_wtime() - st;
                }

                /*
                Поток читает следующий чанк
                */
                #pragma omp section
                {
                    double st = omp_get_wtime();

                    nread1 = getchunk(chunk1, n, m, fp);

                    t_read += omp_get_wtime() - st;
                }
            }
        }

        // t_total += omp_get_wtime() - st;

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


    size_t nread = p - arr;


    free(chunk0);
    free(chunk1);


    printf("read time: %f ms\n", t_read * 1000);
    printf("parse time: %f ms\n", t_parse * 1000);
    // printf("total time: %f ms\n", t_total * 1000);
    // printf("total floats read: %zu\n", nread);

    return nread;
}