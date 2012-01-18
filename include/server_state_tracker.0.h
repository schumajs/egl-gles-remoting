/*! ***************************************************************************
 * \file    server_state_tracker.0.h
 * \brief
 * 
 * \date    January 9, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERVER_STATE_TRACKER_0_H_
#define SERVER_STATE_TRACKER_0_H_

#include <pthread.h>
#include <unistd.h>

#include "transport.h"

struct GVjanitorstate {
    pthread_t      thread;
    GVtransportptr transport;
};

typedef struct GVjanitorstate *GVjanitorstateptr;

typedef GVtransportptr GVdispatcherstateptr;

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] offset
 * \return
 */
int
gvDelJanitorState(size_t offset);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] offset
 * \param  [out] state
 * \return
 */
int
gvGetJanitorState(size_t             offset,
		  GVjanitorstateptr *state);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] offset
 * \param  [in] state
 * \return
 */
int
gvSetJanitorState(size_t            offset,
		  GVjanitorstateptr state);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] state
 * \return
 */
int
gvDelDispatcherState(GVdispatcherstateptr state);

/*! ***************************************************************************
 * \brief
 *
 * \param  [out] state
 * \return
 */
int
gvGetDispatcherState(GVdispatcherstateptr *state);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] state
 * \return
 */
int
gvSetDispatcherState(GVdispatcherstateptr state);

#endif /* SERVER_STATE_TRACKER_0_H_ */
