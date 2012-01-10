/*! ***************************************************************************
 * \file    sleep.c
 * \brief   
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <time.h>

#include "sleep.h"

int
gvslpSleep(int sec, int nsec)
{
    struct timespec sleepTime;
    struct timespec returnTime;

    sleepTime.tv_sec  = sec;
    sleepTime.tv_nsec = nsec;

    nanosleep(&sleepTime, &returnTime);

    return 0;
}
