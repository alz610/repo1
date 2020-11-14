
// #include <stdio.h>

int dprint(const char *fmt, ...);

int float_array_to_str(float *arr, size_t arrsize, size_t strsize, char **str);

int dprint_float_array(float *arr, size_t arrsize);

extern int cols; // точное кол-во чисел в строке
#define OUTPUT_STREAM stderr
