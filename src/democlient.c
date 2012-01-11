
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
    pthread_t thread;

    if (pthread_create(&thread, NULL, threadFunc, "1") != 0)
    {
	perror("pthread_create");
	return -1;
    }

    puts("\n\n\n");

    gvslpSleep(1, 0);

    if (pthread_create(&thread, NULL, threadFunc, "2") != 0)
    {
	perror("pthread_create");
	return -1;
    }

    puts("\n\n\n");

    gvslpSleep(1, 0);

    printf("THREAD PARENT ERROR %i\n", eglGetError());

    gvslpSleep(1, 0);

    return 0;
}
