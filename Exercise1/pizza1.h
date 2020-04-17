/* CONSTANTS */
#define Ncook 6
#define Noven 5
#define Torderlow 1
#define Torderhigh 5
#define Norderlow 1
#define Norderhigh 5
#define Tprep 1
#define Tbake 10

void *order(void *threadId);
void printErrorForProgramArgs(int argc);
void printErrorForMaxNumOfThreads(const int nCust);
void printErrorForNoMemeryAllocate(pthread_t *threads);
void printPThreadError(int rc, int action);