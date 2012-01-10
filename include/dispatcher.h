/*! ***************************************************************************
 * \file    dispatcher.h
 * \brief
 * 
 * \date    January 9, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

/*! ***************************************************************************
 * \namespace gvdis
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include "transport.h"

struct GVDISdispatcher {
    GVTRPtransportptr transport;
};

typedef struct GVDISdispatcher *GVDISdispatcherptr;

/*! ***************************************************************************
 * \brief
 *
 * \param  [out] newDispatcher
 * \param  [in]  transport
 * \return 
 */
int gvdisCreate(GVDISdispatcherptr *newDispatcher, GVTRPtransportptr transport);

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
GVDISdispatcherptr gvdisGetCurrent(void);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] context
 * \return 
 */
int gvdisMakeCurrent(GVDISdispatcherptr context);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] context
 * \return 
 */
int gvdisDestroy(GVDISdispatcherptr context);

#endif  /* DISPATCHER_H_ */
