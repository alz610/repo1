#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <assert.h> 


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

    for (i = 0; i < chunksize; i++)
    {
        if (feof(fp)) break;
        fgets((chunk + linesize * i), linesize, fp);
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
chunksize -- длина чанка (в строках)
    chunk -- читаемый чанк

Вывод:
    нет
*/
int parsechunk(float *p, size_t linesize, size_t chunksize, size_t n_lines_read, char *chunk)
{
    int i_local_chunk = omp_get_thread_num() - 1;
    int n_local_chunks = omp_get_num_threads() - 1;

    size_t local_chunksize = chunksize / n_local_chunks;
    size_t start_line = i_local_chunk * local_chunksize;
    size_t end_line = (i_local_chunk + 1) * local_chunksize;

    // если это последний локальный чанк, то добавить остаток локального чанка
    if (i_local_chunk == n_local_chunks - 1)
        end_line += chunksize % n_local_chunks;


    int i_;
    size_t i_line;
    char *k;
    float num;
    int cols = 5;  // кол-во чисел в строке
    int i = 0;

    float *p_ = p + start_line * cols;

    // #pragma omp for
    for (i_line = start_line; i_line < end_line; i_line++)
    {
        i_ = 0;

        if (i_line > n_lines_read)
            break;

        k = strtok(chunk + linesize * i_line, " ");

        while (k != NULL)
        {
            if (num = atof(k))
            {
                p_[i++] = num;
                i_++;
            }

            k = strtok(NULL, " ");
        }

        assert(cols == i_);
    }


    size_t n_floats_read = i;
    assert(local_chunksize * cols == n_floats_read);

    return n_floats_read;
}

/*
Парсинг файла fp в массив чисел arr.

Ввод:
      arr -- записываемый массив
 linesize -- длина строк чанка
chunksize -- длина чанка (в строках)
       fp -- читаемый файл

Вывод:
nread -- кол-во прочитанных чисел
*/
size_t parsefile(float *arr, size_t linesize, size_t chunksize, FILE *fp)
{
    double t_read = 0, t_parse = 0;
    size_t n_lines_read_0 = 0, n_lines_read_1 = 0;

    char *chunk0 = malloc(chunksize * linesize * sizeof(char));   // предыдущий чанк
    char *chunk1 = malloc(chunksize * linesize * sizeof(char));   // следующий чанк

    float *p = arr;  // позиция в записываемом массиве
  

    {
        double st = omp_get_wtime();
        
        n_lines_read_0 = getchunk(chunk0, linesize, chunksize, fp);
        
        double end = omp_get_wtime();
        t_read += end - st;
    }


    #pragma omp parallel
    {
        size_t n_floats_read;
        arr;

        while (n_lines_read_0 != 0)
        {
            if (omp_get_thread_num() == 0)
            {
                double st = omp_get_wtime();

                n_lines_read_1 = getchunk(chunk1, linesize, chunksize, fp);

                t_read += omp_get_wtime() - st;
            }

            else
            {
                double st = omp_get_wtime();

                n_floats_read = parsechunk(p, linesize, chunksize, n_lines_read_0, chunk0);

                t_parse += omp_get_wtime() - st;

                #pragma omp critical
                    p += n_floats_read;
            }

            #pragma omp barrier

            // swap

            if (omp_get_thread_num() == 0)
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


    printf("read time: %f ms\n", t_read * 1000);
    printf("parse time: %f ms\n", t_parse * 1000);

    return nread;
}