size_t getchunk(char *chunk, size_t n, size_t m, FILE *fp);

int parsechunk(float **p, size_t n, size_t m, char *chunk);

size_t parsefile(float *arr, size_t n, size_t m, FILE *fp);