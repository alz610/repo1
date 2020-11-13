
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <stdarg.h>


#define OUTPUT_STREAM stdout


int dprint(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

#pragma omp critical (dprint_section_name)
{
    // если в параллельной секции, вывод номера потока
    if (omp_in_parallel())
        fprintf(OUTPUT_STREAM, "thread %d: ", omp_get_thread_num());

    vfprintf(OUTPUT_STREAM, fmt, args);
    fflush(OUTPUT_STREAM);
}

    va_end(args);

    return 0;
}


int float_array_to_str(float *arr, size_t arrsize, size_t strsize, char **p_str)
{
    char *str = *p_str;
    int cols = 5;


    char *p = str;

    for (size_t i = 0; i < arrsize; i++)
    {
        p += snprintf(p, strsize, "%e", arr[i]);

        if (!((i + 1) % cols))
            p += snprintf(p, strsize, "\n");
        else
            p += snprintf(p, strsize, " ");
    }


    return 0;
}

int dprint_float_array(float *arr, size_t arrsize)
{
    int cols = 5;

#pragma omp critical (dprint_float_array_section_name)
{
    for (size_t i = 0; i < arrsize; i++)
    {
        fprintf(OUTPUT_STREAM, "%e", arr[i]);

        if (!((i + 1) % cols))
            fprintf(OUTPUT_STREAM, "\n");
        else
            fprintf(OUTPUT_STREAM, " ");
    }
}

    fflush(OUTPUT_STREAM);

    return 0;
}