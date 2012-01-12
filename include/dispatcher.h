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

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include "transport.h"

struct GVdispatcher {
    GVtransportptr transport;
};

typedef struct GVdispatcher *GVdispatcherptr;

/*! ***************************************************************************
 * \brief
 *
 * \param  [out] newDispatcher
 * \param  [in]  transport
 * \return 
 */
int gvCreateDispatcher(GVdispatcherptr *newDispatcher,
		       GVtransportptr   transport);

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
GVdispatcherptr gvGetCurrent(void);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] context
 * \return 
 */
int gvMakeCurrent(GVdispatcherptr dispatcher);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] context
 * \return 
 */
int gvDestroyDispatcher(GVdispatcherptr dispatcher);

#endif  /* DISPATCHER_H_ */
