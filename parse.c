#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <assert.h>
#include "parse.h"


/*
Чтение чанка файла fp в чанк chunk.

Ввод:
    chunk -- записываемый чанк
 linesize -- длина строк чанка
chunksize -- длина чанка (в строках)
       fp -- читаемый файл

Вывод:
n_lines_read -- кол-во прочитанных строк
*/
size_t getchunk(char *chunk, size_t linesize, size_t chunksize, FILE *fp)
{
    size_t i;

    // сброс массива
    for (i = 0; i < chunksize; i++)
        chunk[linesize * i] = '\0';

    for (i = 0; i < chunksize; i++)
    {
        if (feof(fp)) break;
        fgets(chunk + linesize * i, linesize, fp);
    }

    size_t n_lines_read = i;

    return n_lines_read;
}

/*
Функция парсит чанка chunk и записывает распарсенные числа
в позицию *p в записываемом массиве.
Обновляет позицию *p.

Ввод:
             *p -- позиция в записываемом массиве чисел
       linesize -- длина строк чанка
chunksize_lines -- длина чанка (в строках)
          chunk -- читаемый чанк

Вывод:
n_floats_read
*/
int parsechunk(float *p, size_t linesize, size_t chunksize_lines, char *chunk)
{
    char *k;
    float num;

    size_t i = 0;
    int _i = 0;
    int cols = 5;  // кол-во чисел в строке


    for (size_t i_line = 0; i_line < chunksize_lines; i_line++)
    {
        if (DEBUG)
        {
            #pragma omp barrier
            fprintf(stderr, "chunk line read: %s\n", chunk + linesize * i_line);
        }

        k = strtok(chunk + linesize * i_line, " ");
        _i = 0;

        while (k != NULL)
        {
            if (num = atof(k))
            {
                p[i++] = num;
                _i++;
            }

            k = strtok(NULL, " ");
        }

        // assert(cols == _i);
    }


    // assert(chunksize * cols == i);


    size_t n_floats_read = i;

    return n_floats_read;
}

/*
Парсинг файла fp в массив чисел arr.

Ввод:
            arr -- записываемый массив
       linesize -- длина строк чанка
chunksize_lines -- длина чанка (в строках)
             fp -- читаемый файл

Вывод:
nread -- кол-во прочитанных чисел
*/
size_t parsefile(float *arr, size_t linesize, size_t chunksize_lines, FILE *fp)
{
    double t_read = 0, t_parse = 0;
    size_t n_lines_read_0 = 0, n_lines_read_1 = 0;

    char *chunk0 = malloc(chunksize_lines * linesize * sizeof(char));   // предыдущий чанк
    char *chunk1 = malloc(chunksize_lines * linesize * sizeof(char));   // следующий чанк

    float *p = arr;  // позиция в записываемом массиве
    int n_subchunks, last_thread_num;
    size_t *n_floats_read;


    {
        double st = omp_get_wtime();

        n_lines_read_0 = getchunk(chunk0, linesize, chunksize_lines, fp);

        t_read += omp_get_wtime() - st;
    }

    // int _i = 0;
    int cols = 5;  // кол-во чисел в строке

    #pragma omp parallel // shared(p, n_subchunks, n_floats_read, last_thread_num)
    {
        // arr; // для отладки
        last_thread_num = omp_get_num_threads() - 1;
        n_subchunks = omp_get_num_threads() - 1;


        #pragma omp master
            n_floats_read = malloc(n_subchunks * sizeof(size_t));

        #pragma omp barrier


        while (n_lines_read_0 != 0)
        {
            if (omp_get_thread_num() == 0)
            {
                double st = omp_get_wtime();

                n_lines_read_1 = getchunk(chunk1, linesize, chunksize_lines, fp);

                #pragma omp critical
                    t_read += omp_get_wtime() - st;
            }

            else
            {
            // #pragma omp critical
            // {
                if (DEBUG)
                {
                    #pragma omp barrier
                    fprintf(stderr, "\n\nthread %d:\n", omp_get_thread_num());
                }

                int i_subchunk = omp_get_thread_num() - 1;

                size_t subchunksize_lines = chunksize_lines / n_subchunks;

                char *subchunk = chunk0 + i_subchunk * subchunksize_lines * linesize;

                // если это последний локальный чанк, то добавить остаток локального чанка
                if (i_subchunk == n_subchunks - 1)
                    subchunksize_lines += chunksize_lines % n_subchunks;

                if (DEBUG)
                {
                    #pragma omp barrier
                    fprintf(stderr, "subchunk: offset %ld, size_lines %ld\n\n",
                            (size_t) (subchunk - chunk0), subchunksize_lines);
                }

                size_t linesize_floats = 32;

                float *sub_p = malloc(subchunksize_lines * linesize_floats * sizeof(float));


                double st = omp_get_wtime();

                n_floats_read[i_subchunk] = parsechunk(sub_p, linesize, subchunksize_lines, subchunk);

                #pragma omp critical
                    t_parse += omp_get_wtime() - st;


                #pragma omp barrier


                if (DEBUG)
                {
                    #pragma omp barrier
                    fprintf(stderr, "floats read: %zu\n", n_floats_read[i_subchunk]);

                    /* float values were successfully read */
                    for (size_t i = 0; i < n_floats_read[i_subchunk]; i++)
                    {
                        #pragma omp barrier
                        fprintf(stderr, "%e", sub_p[i]);

                        if (!((i + 1) % cols))
                        {
                            #pragma omp barrier
                            fprintf(stderr, "\n");
                        }
                        else
                        {
                            #pragma omp barrier
                            fprintf(stderr, " ");
                        }
                    }
                }


                size_t write_offset = 0;

                for (size_t j = 0; j < i_subchunk; j++)
                    write_offset += n_floats_read[j];

                if (DEBUG)
                {
                    #pragma omp barrier
                    fprintf(stderr, "write offset: %ld\n", write_offset);
                }
                
                if (TEST)
                    assert(write_offset == subchunksize_lines * cols * i_subchunk);

                memcpy(p + write_offset, sub_p, n_floats_read[i_subchunk] * sizeof(float));


                #pragma omp critical
                    p += n_floats_read[i_subchunk];
                
                #pragma omp critical
                    fflush(stderr);
            // }
            }

            #pragma omp barrier

            // swap

            #pragma omp master
            {
                {
                    char *temp = chunk0;
                    chunk0 = chunk1;
                    chunk1 = temp;
                }

                {
                    size_t temp = n_lines_read_0;
                    n_lines_read_0 = n_lines_read_1;
                    n_lines_read_1 = temp;
                }
            }

            #pragma omp barrier
        }
    }


    size_t nread = p - arr;


    free(chunk0);
    free(chunk1);


    if (VERBOSE)
    {
        printf("read time: %f ms\n", t_read * 1000);
        printf("parse time: %f ms\n", t_parse * 1000);
    }

    return nread;
}