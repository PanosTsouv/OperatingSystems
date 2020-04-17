#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <float.h>
#include <pthread.h>
#include "pizza1.h"

#define DEBUG 0

pthread_mutex_t gSharedVariableNcook = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gSharedVariableNoven = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gSharedPrintScreen = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gFinishTimes = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gPreparePhase = PTHREAD_COND_INITIALIZER;
pthread_cond_t gBakePhase = PTHREAD_COND_INITIALIZER;
int numberOfCook = 0;
int numberOfOven = 0;
double maxFinishTime = 0;
double averageFinishTime = 0;
double maxOrderFinishTime = 0;
double minOrderFinishTime = DBL_MAX;

struct timespec mainStart;

void *order(void *threadId)
{
    struct timespec start;
    struct timespec finish;
    clock_gettime(CLOCK_REALTIME, &start);
    int orderTime;
    int *tid;
    int numberOfPizzas;
    int prepareTime;
    int rc;

    tid = (int *)threadId;
    numberOfPizzas = (rand() % Norderhigh) + Norderlow;

    rc = pthread_mutex_lock(&gSharedVariableNcook);//lock gSharedVariableNcook
    printPThreadError(rc, 3);
        while(numberOfCook == Ncook)
        {
            rc = pthread_cond_wait(&gPreparePhase, &gSharedVariableNcook);
            printPThreadError(rc, 5);
        }
        numberOfCook++;
    rc = pthread_mutex_unlock(&gSharedVariableNcook);//unlock
    printPThreadError(rc, 4);

    prepareTime = numberOfPizzas * Tprep;
    sleep(prepareTime);

    rc = pthread_mutex_lock(&gSharedVariableNoven);//lock gSharedVariableNoven
    printPThreadError(rc, 3);
        while(numberOfOven == Noven)
        {
            rc = pthread_cond_wait(&gBakePhase, &gSharedVariableNoven);
            printPThreadError(rc, 5);
        }
        numberOfOven++;
    rc = pthread_mutex_unlock(&gSharedVariableNoven);//unlock
    printPThreadError(rc, 4);

    sleep(Tbake);

    rc = pthread_mutex_lock(&gSharedVariableNoven);//lock gSharedVariableNoven
    printPThreadError(rc, 3);
        numberOfOven--;
        rc = pthread_cond_signal(&gBakePhase);
        printPThreadError(rc, 6);
    rc = pthread_mutex_unlock(&gSharedVariableNoven);//unlock
    printPThreadError(rc, 4);

    rc = pthread_mutex_lock(&gSharedVariableNcook);//lock gSharedVariableNcook
    printPThreadError(rc, 3);
        numberOfCook--;
        rc = pthread_cond_signal(&gPreparePhase);
        printPThreadError(rc, 6);
    rc = pthread_mutex_unlock(&gSharedVariableNcook);//unlock
    printPThreadError(rc, 4);

    clock_gettime(CLOCK_REALTIME, &finish);

	rc = pthread_mutex_lock(&gSharedPrintScreen);//lock gSharedVariableNcook
    printPThreadError(rc, 3);
        orderTime = finish.tv_sec - start.tv_sec;
        printf("Order with number %d was prepared in %d minutes!\n", *tid, orderTime);
        if(DEBUG)
            printf("Order with number %d finish in %f minutes from main's start!\n", *tid, (double)(finish.tv_sec - mainStart.tv_sec));
    rc = pthread_mutex_unlock(&gSharedPrintScreen);//unlock
    printPThreadError(rc, 4);

    rc = pthread_mutex_lock(&gFinishTimes);//lock gFinishTimes
    printPThreadError(rc, 3);
        maxFinishTime = maxFinishTime + orderTime;
        if(orderTime > maxOrderFinishTime)
        {
            maxOrderFinishTime = orderTime;
        }
        if(orderTime < minOrderFinishTime)
        {
            minOrderFinishTime = orderTime;
        }
    rc = pthread_mutex_unlock(&gFinishTimes);//unlock
    printPThreadError(rc, 4);
	pthread_exit(tid);
}

int main(int argc, char *argv[]){
    clock_gettime(CLOCK_REALTIME, &mainStart);
    printErrorForProgramArgs(argc);

    const int nCust = atoi(argv[1]);
    printErrorForMaxNumOfThreads(nCust);
    int rc;
   	int threadCount;
	int idArray[nCust];
    int mainWaitingTime;
    int creationTime = 0;

    if(DEBUG)
        printf("Main: We will create %d threads.\n", nCust);

    srand((unsigned int)atoi(argv[2]));

    pthread_t *threads;
	threads = malloc(nCust * sizeof(pthread_t));
    printErrorForNoMemeryAllocate(threads);
	
   	for(threadCount = 0; threadCount < nCust; threadCount++) 
    {
		idArray[threadCount] = threadCount + 1;
        if (threadCount)
        {
            mainWaitingTime = (rand() % Torderhigh) + Torderlow;
            creationTime = creationTime + mainWaitingTime;
            sleep(mainWaitingTime);
            if(DEBUG)
                printf("Main: Creating thread %d Time: %d\n", idArray[threadCount], creationTime);
        }
        else
        {
            if(DEBUG)
                printf("Main: Creating thread %d Time: %d\n", idArray[threadCount], creationTime);
        }
    	rc = pthread_create(&threads[threadCount], NULL, order, &idArray[threadCount]);
        printPThreadError(rc, 1);
    }

    void *status;
	for (threadCount = 0; threadCount < nCust; threadCount++) {
		rc = pthread_join(threads[threadCount], &status);
        printPThreadError(rc, 2);
        if(DEBUG)
		    printf("Main: Thread %d returned %d as status code.\n", idArray[threadCount], (*(int *)status));
	}

    averageFinishTime = maxFinishTime / nCust;
    printf ("Main(): Total time for orders to finish was %f.\n", maxFinishTime);
    printf ("Main(): Average time for orders to finish %f.\n", averageFinishTime);
    printf ("Main(): Maximum order's finish time was %f.\n", maxOrderFinishTime);
    printf ("Main(): Minimum order's finish time was %f.\n", minOrderFinishTime);


    free(threads);
  	rc = pthread_mutex_destroy(&gSharedVariableNcook);
	printPThreadError(rc, 7);
    rc = pthread_mutex_destroy(&gSharedVariableNoven);
	printPThreadError(rc, 7);
    rc = pthread_mutex_destroy(&gSharedPrintScreen);
	printPThreadError(rc, 7);
 	rc = pthread_cond_destroy(&gPreparePhase);
	printPThreadError(rc, 8);
    rc = pthread_cond_destroy(&gBakePhase);
	printPThreadError(rc, 8);

    return 1;
}

void printErrorForProgramArgs(int argc)
{
    if (argc != 3)
    {
		printf("ERROR: the program should take two argument, the number of threads to create and the seed of random function!\n");
		exit(-1);
	}
}

void printErrorForMaxNumOfThreads(const int nCust)
{
    if (nCust < 0)
    {
		printf("ERROR: the number of threads to run should be a positive number. Current number given %d.\n", nCust);
		exit(-1);
	}
}

void printErrorForNoMemeryAllocate(pthread_t *threads)
{
    if (threads == NULL)
    {
		printf("NOT ENOUGH MEMORY!\n");
		exit(-1);
	}
}

void printPThreadError(int rc, int action)
{
    if(rc != 0)
    {
        switch(action)
        {
            case 1 : 
                printf("ERROR: return code from pthread_create() is %d\n", rc);
                exit(-1);
            case 2 : 
                printf("ERROR: return code from pthread_join() is %d\n", rc);
                exit(-1);
            case 3 :
                printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		        pthread_exit(&rc);
            case 4 :
                printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		        pthread_exit(&rc);
            case 5 :
                printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
                pthread_exit(&rc);
            case 6 :
                printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
                pthread_exit(&rc);
            case 7 :
                printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		        exit(-1);
            case 8 :
                printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		        exit(-1);
        }
    }
}