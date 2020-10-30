//thread check
#define TCHK(exp) do { \
    if ((errno = (exp)) > 0) \
        {perror(#exp); exit(1);} \
    } while(0)

//memory check
#define MCHK(exp) do { \
    if ((exp) == NULL) \
        {perror(#exp); exit(1);} \
    } while(0)

//primitiv check
#define PCHK(exp) do { \
    if ((exp) == -1) \
        {perror(#exp); exit(1);} \
    } while(0)
