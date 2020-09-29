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

    /* далее идет набросок кода с двумя параллельными потоками */

    // чтение чанка 0 из строк текстовика

    for (size_t i = 0; i < count; i++)    
    {
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                // чтение чанка i + 1
            }
            #pragma omp section
            {
                // парсинг чанка i
            }
        }
    }

    /* n-1 float values were successfully read */
    for (i=0; i<n-1; i++)
        printf("fval[%d]=%f\n", i, fval[i]);

    fclose(fp);

    return 0;
}