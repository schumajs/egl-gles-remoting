/*! ***************************************************************************
 * \file    server_dispatcher.h
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

typedef void (*GVdispatchfunc)(void);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] dispatcher
 * \param  [in] jumpTable
 * \param  [in] joinThread
 * \return
 */
int gvDispatchLoop(GVdispatcherptr   dispatcher,
		   GVDISdispatchfunc jumpTable[],
		   int               joinThread);

#endif /* SERVER_DISPATCHER_H_ */
