size_t getchunk(char *chunk, size_t linesize, size_t chunksize, FILE *fp);

int parsechunk(float *p, size_t linesize, size_t chunksize, char *chunk);

size_t parsefile(float *arr, size_t linesize, size_t chunksize, FILE *fp);

extern int DEBUG_LVL, TEST;
extern int N_PARSETHREADS;  // кол-во потоков парсинга чанка во вложенной параллельной секции
