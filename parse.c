#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <assert.h>
#include "parse.h"


// extern int dprint(const char *fmt, ...);

int dprint(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

#pragma omp critical (dprint_section_name)
{
    fprintf(stderr, "thread %d: ", omp_get_thread_num());
    vfprintf(stderr, fmt, args);
    fflush(stderr);
}

    va_end(args);
}


int print_float_array(float* arr, size_t len)
{
    int cols = 5;

    for (size_t i = 0; i < len; i++)
    {
        dprint("%e", arr[i]);

        if (!((i + 1) % cols))
            dprint("\n");
        else
            dprint(" ");
    }

    dprint("\n\n");
    
    return 0;
}


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
                assert(cols == n_parsed_nums_in_line);
        }
    }


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
    double t_parse_single = 0;
    size_t n_lines_read = 0;  // кол-во прочитанных строк из файла для чанка

    char *chunk0 = calloc(chunksize_lines * linesize, sizeof(char));   // предыдущий чанк
    char *chunk1 = calloc(chunksize_lines * linesize, sizeof(char));   // следующий чанк


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
 
    // массив c размером `n_floats_read` служит для записи кол-ва чисел, распарсенных потоками парсинга
    size_t *n_floats_read;  

    // setbuf(stdout, NULL);

    /*
    Параллельная секция с 2 потоками:
      - нулевой поток, читающий чанк из файла;
      - первый поток с вложенной параллельной секцией с `N_PARSETHREADS` потоков парсинга чанка.
    */
    #pragma omp parallel num_threads(N_PARSETHREADS + 1)
    {
        int n_threads = omp_get_num_threads();
        int i_thread = omp_get_thread_num();

        int n_parsethreads = n_threads - 1;
        int i_parsethread = i_thread - 1;


#pragma omp master
        n_floats_read = malloc(n_parsethreads * sizeof(size_t));


        // размер подчанка потока в строках
        size_t subchunksize_lines = chunksize_lines / n_parsethreads;

        // если это последний подчанк, то добавить остаток подчанка
        if (i_parsethread == n_parsethreads - 1)
            subchunksize_lines += chunksize_lines % n_parsethreads;

        // распарсенные числа подчанка
        size_t linesize_floats = 32;

        float *sub_p = malloc(subchunksize_lines * linesize_floats * sizeof(float));


        int counter = 0;

        do {
            counter++;
            printf("thread %d, begin parallel loop, counter = %d\n", i_thread, counter);

#pragma omp barrier

            // если нулевой поток
            if (i_thread == 0)
            {
                double st = omp_get_wtime();

                // чтение следующего чанка из файла fp в массив chunk1
                n_lines_read = getchunk(chunk1, linesize, chunksize_lines, fp);

                t_read += omp_get_wtime() - st;
            }


            if (i_thread > 0)
            {
                // subchunk -- начало подчанка
                char *subchunk = chunk0 + i_parsethread * subchunksize_lines * linesize;

#pragma omp single nowait
                t_parse_single -= omp_get_wtime();

                n_floats_read[i_parsethread] = parsechunk(sub_p, linesize, subchunksize_lines, subchunk);

#pragma omp critical (name0)
{
                printf("thread %d, parsed floats output:\n", i_thread);
                size_t len = n_floats_read[i_parsethread];
                print_float_array(sub_p, len);
}


#pragma omp single nowait
                t_parse_single += omp_get_wtime();

#pragma omp single nowait
                t_parse += t_parse_single;
            }


#pragma omp barrier

            if (i_thread > 0)
            {
                // вычисление смещения write_offset положения записи p
                // распарсенных чисел подчанка subchunk
                // в массив распарсенных чисел arr

                size_t write_offset = 0;

                for (size_t j = 0; j < i_parsethread; j++)
                    write_offset += n_floats_read[j];

                // запись массива sub_p по смещению write_offset положения записи p
                // в массив распарсенных чисел arr

                memcpy(p + write_offset, sub_p, n_floats_read[i_parsethread] * sizeof(float));
            }


#pragma omp barrier

            if (i_thread > 0)
            {
                // обновление позиции p

#pragma omp critical (name1)
                p += n_floats_read[i_parsethread];
            }


#pragma omp barrier

#pragma omp master
{
            // замена

            char *temp = chunk0;
            chunk0 = chunk1;
            chunk1 = temp;
}

#pragma omp barrier

        // когда есть новые прочитанные строки в файле fp
        } while (n_lines_read);
    }



    size_t nread = p - arr;

    free(chunk0);
    free(chunk1);


    printf("read time: %f ms\n", t_read * 1000);
    printf("parse time: %f ms\n", t_parse * 1000);


    return nread;
}
