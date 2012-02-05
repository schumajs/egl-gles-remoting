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
 * \return
 */
unsigned long
gvDispatchLoop(GVtransportptr transport,
	       GVdispatchfunc jumpTable[]);

#endif /* SERVER_DISPATCHER_H_ */
