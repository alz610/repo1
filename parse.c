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

    // сброс массива, запись нуль-терминаторов в строки чанка
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

Ввод:
             *p -- позиция в записываемом массиве чисел
       linesize -- длина строк чанка
chunksize_lines -- длина чанка (в строках)
          chunk -- читаемый чанк

Вывод:
n_floats_read -- кол-во распарсенных чисел в чанке
*/
int parsechunk(float *p, size_t linesize, size_t chunksize_lines, char *chunk)
{
    size_t i = 0; // смещение положения записи p
    int cols = 5; // точное кол-во чисел в строке

    for (size_t i_line = 0; i_line < chunksize_lines; i_line++)
    {
        int n_parsed_nums_in_line = 0; // кол-во распарсенных чисел в строке line

        char *line = chunk + linesize * i_line;


        if (DEBUG_LVL >= 2)
        {
            #pragma omp barrier
            fprintf(stderr, "chunk line read: %s\n", line);
        }


        // извлечь первую подстроку строки line, разделенную пробелом
        char *substring = strtok(line, " ");

        while (substring != NULL)
        {
            float num = atof(substring);

            // если распарсено число в подстроке
            if (num)
            {
                p[i++] = num;

                if (TEST)
                    n_parsed_nums_in_line++;
            }

            // извлечь следующую подстроку строки line, разделенную пробелом
            substring = strtok(NULL, " ");
        }

        if (TEST)
        {
            // если что-либо распарсено в строке line, то проверка, верное ли кол-во распарсенных чисел
            if (*line != '\0')
            {
                // assert(cols == n_parsed_nums_in_line);
                if (!(cols == n_parsed_nums_in_line))
                {
                    fprintf(stderr, "thread %d: fail: 'cols == n_parsed_nums_in_line'.\n",
                            omp_get_thread_num());  fflush(stderr);
                    fprintf(stderr, "thread %d: cols=%d; n_parsed_nums_in_line=%ld\n",
                            omp_get_thread_num(), cols, n_parsed_nums_in_line);  fflush(stderr);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }


    // ассерт не наладен

    // if (TEST)
    //     assert(chunksize * cols == i);


    if (DEBUG_LVL >= 2)
        fprintf(stderr, "\n\n");


    // кол-во распарсенных чисел в чанке
    size_t n_floats_read = i;

    return n_floats_read;
}


/*
Парсинг файла fp в массив чисел arr.

Ввод:
            arr -- записываемый массив чисел
       linesize -- длина строк чанка
chunksize_lines -- длина чанка (в строках)
             fp -- читаемый файл

Вывод:
nread -- кол-во прочитанных чисел
*/
size_t parsefile(float *arr, size_t linesize, size_t chunksize_lines, FILE *fp)
{
    int cols = 5;  // кол-во чисел в строке файла
    double t_read = 0, t_parse = 0;  // время чтения из файла чанков и парсинга чанков
    size_t n_lines_read = 0;  // кол-во прочитанных строк из файла для чанка

    char *chunk0 = malloc(chunksize_lines * linesize * sizeof(char));   // предыдущий чанк
    char *chunk1 = malloc(chunksize_lines * linesize * sizeof(char));   // следующий чанк


    /*
    Структура чанков:
    смещение     | данные
    ------------------------------------------------------------
    linesize * 0 | начало строки, меньшей linesize
    linesize * 1 | начало строки, меньшей linesize
    linesize * 2 | начало строки, меньшей linesize
    ...
    linesize * chunksize_lines | начало строки, меньшей linesize
    */


    float *p = arr;  // позиция в записываемом массиве чисел
    int n_parsethreads = N_PARSETHREADS;  // кол-во потоков парсинга чанка во вложенной параллельной секции

    fprintf(stderr, "parse threads: %d\n", n_parsethreads);
 
    // массив c размером `n_parsethreads` служит для записи кол-ва чисел, распарсенных потоками парсинга
    size_t *n_floats_read;  

    // включение вложенных параллельных секций
    omp_set_nested(1);

    /*
    Параллельная секция с 2 потоками:
      - нулевой поток, читающий чанк из файла;
      - первый поток с вложенной параллельной секцией с `n_parsethreads` потоков парсинга чанка.
    */
    #pragma omp parallel num_threads(2)
    {
        // область кода для исполения главным потоком (нулевой поток)
        #pragma omp master
        {
            // выделение массива для хранения кол-ва чисел, распарсенных потоками парсинга
            n_floats_read = malloc(n_parsethreads * sizeof(size_t));


            double st = omp_get_wtime();

            // чтение первого чанка из файла fp и запись его в массив chunk0
            int n_lines_read = getchunk(chunk0, linesize, chunksize_lines, fp);

            t_read += omp_get_wtime() - st;
        }

        fprintf(stderr, "thread %d before birrier 1\n", omp_get_thread_num()); fflush(stderr);
#pragma omp barrier
        fprintf(stderr, "thread %d after birrier 1\n", omp_get_thread_num()); fflush(stderr);

        int counter = 0;
        // когда есть новые прочитанные строки в файле fp
        while (1)
        {
#pragma omp barrier
            counter++;

            fprintf(stderr, "BEGIN_LOOP thread %d n_lines_read = %d counter=%d\n", omp_get_thread_num(), n_lines_read, counter); fflush(stderr);

            // если нулевой поток
            if (omp_get_thread_num() == 0)
            {
                double st = omp_get_wtime();

                // чтение следующего чанка из файла fp в массив chunk1
                n_lines_read = getchunk(chunk1, linesize, chunksize_lines, fp);
                //#pragma omp flush (n_lines_read)

                t_read += omp_get_wtime() - st;
                /*
                if (!n_lines_read)
                {
                  fprintf(stderr, "thread %d exit loop !!!!\n", omp_get_thread_num()); fflush(stderr);
                  break;
                }*/
            }

            fprintf(stderr, "thread %d n_lines_read = %d\n", omp_get_thread_num(), n_lines_read); fflush(stderr);


            // если первый поток
            if (omp_get_thread_num() == 1)
            {
                // вложенная параллельная секция с `n_parsethreads` потоками парсинга
                #pragma omp parallel num_threads(n_parsethreads)
                {
                    int i_thread = omp_get_thread_num();
                    int nnn = omp_get_num_threads();

                    if (DEBUG_LVL >= 2)
                    {
                        #pragma omp barrier
                        fprintf(stderr, "thread: %d\n\n\n", i_thread);
                    }

                    int i_subchunk = i_thread;
                    int n_subchunks = n_parsethreads;

                    // размер подчанка потока в строках
                    size_t subchunksize_lines = chunksize_lines / n_subchunks;

                    // subchunk -- начало подчанка
                    char *subchunk = chunk0 + i_subchunk * subchunksize_lines * linesize;


                    // если это последний подчанк, то добавить остаток подчанка
                    if (i_subchunk == n_subchunks - 1)
                        subchunksize_lines += chunksize_lines % n_subchunks;


                    if (DEBUG_LVL >= 2)
                    {
                        #pragma omp barrier
                        fprintf(stderr, "subchunk: offset %ld, size_lines %ld\n\n\n",
                                (size_t) (subchunk - chunk0), subchunksize_lines);
                    }


                    size_t linesize_floats = 32;

                    // распарсенные числа подчанков
                    float *sub_p = malloc(subchunksize_lines * linesize_floats * sizeof(float));


                    double st = omp_get_wtime();

                    n_floats_read[i_subchunk] = parsechunk(sub_p, linesize, subchunksize_lines, subchunk);

                    #pragma omp critical
                        t_parse += omp_get_wtime() - st;
 

                    if (DEBUG_LVL >= 2)
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

                    #pragma omp barrier

                    // вычисление смещения write_offset положения записи p
                    // распарсенных чисел подчанка subchunk
                    // в массив распарсенных чисел arr

                    size_t write_offset = 0;

                    for (size_t j = 0; j < i_subchunk; j++)
                        write_offset += n_floats_read[j];


                    if (DEBUG_LVL >= 2)
                    {
                        #pragma omp barrier
                        fprintf(stderr, "thread %d: write offset: %ld\n\n\n", i_thread, write_offset);
                    }


                    // ассерт не наладен

                    // if (TEST)
                    //     assert(write_offset == subchunksize_lines * cols * i_subchunk);


                    // запись массива sub_p по смещению write_offset положения записи p
                    // в массив распарсенных чисел arr

                    // #pragma omp barrier

                    memcpy(p + write_offset, sub_p, n_floats_read[i_subchunk] * sizeof(float));


                    // обновление позиции p
                    fprintf(stderr, "thread %d before birrier 2\n", omp_get_thread_num()); fflush(stderr);
#pragma omp barrier
                    fprintf(stderr, "thread %d after birrier 2\n", omp_get_thread_num()); fflush(stderr);

                    #pragma omp critical
                        p += n_floats_read[i_subchunk];
                    

                    if (DEBUG_LVL >= 2)
                    {
                        #pragma omp barrier
                        fprintf(stderr, "\n");
                    }
                }///////////// inner parallel loop
            }


            fprintf(stderr, "thread %d of %d before inner barrier 1\n", omp_get_thread_num(), omp_get_num_threads()); fflush(stderr);
            #pragma omp barrier
            fprintf(stderr, "thread %d of %d after inner barrier 1\n", omp_get_thread_num(), omp_get_num_threads()); fflush(stderr);

            #pragma omp master
            {
                // замена

                char *temp = chunk0;
                chunk0 = chunk1;
                chunk1 = temp;
            }

            fprintf(stderr, "thread %d before inner barrier 2\n", omp_get_thread_num()); fflush(stderr);
            #pragma omp barrier
            fprintf(stderr, "thread %d after inner barrier 2\n", omp_get_thread_num()); fflush(stderr);

            fprintf(stderr, "END_LOOP thread %d n_lines_read = %d counter=%d\n", omp_get_thread_num(), n_lines_read, counter); fflush(stderr);


            if (!n_lines_read)
              { 
                fprintf(stderr, "thread %d exit loop !!!!\n", omp_get_thread_num()); fflush(stderr);
                break;
              }
        }
    }

    //omp_set_nested(0);


    size_t nread = p - arr;


    free(chunk0);
    free(chunk1);


    // if (DEBUG_LVL >= 2)
    // {
    //     fprintf(stderr, "total floats read: %zu\n", nread);
    // }


    // if (DEBUG_LVL >= 2)
    // {
    //     /* float values were successfully read */
    //     for (size_t i = 0; i < nread; i++)
    //     {
    //         fprintf(stderr, "%e", arr[i]);

    //         if (!((i + 1) % cols))
    //             fprintf(stderr, "\n");
    //         else
    //             fprintf(stderr, " ");
    //     }

    //     fprintf(stderr, "\n\n");
    // }


    if (DEBUG_LVL >= 1)
    {
        fprintf(stderr, "read time: %f ms\n", t_read * 1000);
        fprintf(stderr, "parse time: %f ms\n", t_parse * 1000);
    }

    return nread;
}
