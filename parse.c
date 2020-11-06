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
        char *line = chunk + linesize * i_line;

        if (DEBUG)
        {
            #pragma omp barrier
            fprintf(stderr, "chunk line read: %s\n", line);
        }

        k = strtok(line, " ");
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

        if (*line != '\0')
            assert(cols == _i);
    }


    // assert(chunksize * cols == i);

    if (DEBUG)
        fprintf(stderr, "\n\n");

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
    size_t n_lines_read = 0;

    char *chunk0 = malloc(chunksize_lines * linesize * sizeof(char));   // предыдущий чанк
    char *chunk1 = malloc(chunksize_lines * linesize * sizeof(char));   // следующий чанк

    float *p = arr;  // позиция в записываемом массиве
    int n_parsethreads = 3;
    size_t *n_floats_read;


    {
        double st = omp_get_wtime();

        n_lines_read = getchunk(chunk0, linesize, chunksize_lines, fp);

        t_read += omp_get_wtime() - st;
    }


    // int _i = 0;
    int cols = 5;  // кол-во чисел в строке


    omp_set_nested(1);

    #pragma omp parallel num_threads(2) // shared(p, n_subchunks, n_floats_read, last_thread_num)
    {
        #pragma omp master
            n_floats_read = malloc(n_parsethreads * sizeof(size_t));

        #pragma omp barrier


        while (n_lines_read != 0)
        {
            if (omp_get_thread_num() == 0)
            {
                double st = omp_get_wtime();

                n_lines_read = getchunk(chunk1, linesize, chunksize_lines, fp);

                // #pragma omp critical
                    t_read += omp_get_wtime() - st;
            }

            else
            {
                #pragma omp parallel num_threads(n_parsethreads)
                {
                    int i_thread = omp_get_thread_num();

                    if (DEBUG)
                    {
                        #pragma omp barrier
                        fprintf(stderr, "thread: %d\n\n\n", i_thread);
                    }

                    int i_subchunk = i_thread;
                    int n_subchunks = n_parsethreads;

                    size_t subchunksize_lines = chunksize_lines / n_subchunks;

                    char *subchunk = chunk0 + i_subchunk * subchunksize_lines * linesize;

                    // если это последний локальный чанк, то добавить остаток локального чанка
                    if (i_subchunk == n_subchunks - 1)
                        subchunksize_lines += chunksize_lines % n_subchunks;


                    if (DEBUG)
                    {
                        #pragma omp barrier
                        fprintf(stderr, "subchunk: offset %ld, size_lines %ld\n\n\n",
                                (size_t) (subchunk - chunk0), subchunksize_lines);
                    }


                    size_t linesize_floats = 32;

                    float *sub_p = malloc(subchunksize_lines * linesize_floats * sizeof(float));


                    double st = omp_get_wtime();

                    n_floats_read[i_subchunk] = parsechunk(sub_p, linesize, subchunksize_lines, subchunk);

                    #pragma omp critical
                        t_parse += omp_get_wtime() - st;
 

                    if (DEBUG)
                    {
                        #pragma omp barrier

                        #pragma omp critical
                        {
                            fprintf(stderr, "floats read: %zu\n", n_floats_read[i_subchunk]);

                            /* float values were successfully read */
                            for (size_t i = 0; i < n_floats_read[i_subchunk]; i++)
                            {
                                fprintf(stderr, "%e", sub_p[i]);

                                if (!((i + 1) % cols))
                                    fprintf(stderr, "\n");
                                else
                                    fprintf(stderr, " ");
                            }

                            fprintf(stderr, "\n\n");
                        }
                    }


                    size_t write_offset = 0;

                    for (size_t j = 0; j < i_subchunk; j++)
                        write_offset += n_floats_read[j];


                    if (DEBUG)
                    {
                        #pragma omp barrier
                        fprintf(stderr, "thread %d: write offset: %ld\n\n\n", i_thread, write_offset);
                    }
                    
                    if (TEST)
                        assert(write_offset == subchunksize_lines * cols * i_subchunk);


                    #pragma omp barrier

                    memcpy(p + write_offset, sub_p, n_floats_read[i_subchunk] * sizeof(float));


                    #pragma omp critical
                        p += n_floats_read[i_subchunk];
                    

                    if (DEBUG)
                    {
                        #pragma omp barrier
                        fprintf(stderr, "\n");
                    }
                }
            }


            #pragma omp barrier


            #pragma omp master
            {
                // swap

                char *temp = chunk0;
                chunk0 = chunk1;
                chunk1 = temp;
            }


            #pragma omp barrier
        }
    }


    size_t nread = p - arr;


    free(chunk0);
    free(chunk1);


    if (DEBUG)
    {
        fprintf(stderr, "total floats read: %zu\n", nread);

        /* float values were successfully read */
        for (size_t i = 0; i < nread; i++)
        {
            fprintf(stderr, "%e", arr[i]);

            if (!((i + 1) % cols))
                fprintf(stderr, "\n");
            else
                fprintf(stderr, " ");
        }
    }


    if (VERBOSE)
    {
        fprintf(stderr, "read time: %f ms\n", t_read * 1000);
        fprintf(stderr, "parse time: %f ms\n", t_parse * 1000);
    }

    return nread;
}