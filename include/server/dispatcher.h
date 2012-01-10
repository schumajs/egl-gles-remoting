/*! ***************************************************************************
 * \file    dispatcher.h
 * \brief
 * 
 * \date    January 9, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERVER_DISPATCHER_H_
#define SERVER_DISPATCHER_H_

#include "transport.h"

#include "../dispatcher.h" 

typedef void (*GVDISfunc)(void);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] dispatcher
 * \param  [in] jumpTable
 * \param  [in] join
 * \return
 */
int gvdisDispatchLoop(GVDISdispatcherptr dispatcher, GVDISfunc jumpTable[], int join);

#endif /* SERVER_DISPATCHER_H_ */
