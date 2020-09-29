#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE * fp;
    float fval[1000];
    int n, i;

    fp = fopen("message.txt", "r");
    if (fp == NULL) {
        printf("failed to open file\n");
        return 1;
    }

    #pragma omp parallel // specify the code between the curly brackets is part of an OpenMP parallel section.
    {
        char str[10];

        n = 0;
        while (1) {
            fscanf(fp, "%10s", str);
            if (*str == '/') break;
            if (fscanf(fp, "%f", &fval[n++]) == EOF) break;
        }
    }

    /* n-1 float values were successfully read */
    for (i=0; i<n-1; i++)
        printf("fval[%d]=%f\n", i, fval[i]);

    fclose(fp);

    return 0;
}