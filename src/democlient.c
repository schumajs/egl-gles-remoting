
#define _MULTI_THREADED
#include <pthread.h>
#include <stdio.h>
#include <EGL/egl.h>

#include "dispatcher.h"
#include "lock.h"
#include "sleep.h"

static void
*threadFunc(void *threadArg)
{ 
    printf("THREAD %s ERROR %i\n", (char *)threadArg, eglGetError());

    return NULL;
}

int main()
{
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;

    printf("THREAD PARENT ERROR %i\n", eglGetError());

    /* gvslpSleep(1, 0); */

    if (pthread_create(&thread1, NULL, threadFunc, "1") != 0)
    {
	perror("pthread_create");
	return -1;
    }

    /* gvslpSleep(1, 0); */

    if (pthread_create(&thread2, NULL, threadFunc, "2") != 0)
    {
	perror("pthread_create");
	return -1;
    }

    /* gvslpSleep(1, 0); */

    if (pthread_create(&thread3, NULL, threadFunc, "3") != 0)
    {
	perror("pthread_create");
	return -1;
    }

    gvslpSleep(2, 0);

    return 0;
}
