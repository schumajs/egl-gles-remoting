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

struct GVoffsetstate {
    pthread_t      thread;
    GVtransportptr transport;
};

typedef struct GVoffsetstate *GVoffsetstateptr;

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] offset
 * \return
 */
int
gvDelOffsetState(size_t offset);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] offset
 * \return
 */
GVoffsetstateptr
gvGetOffsetState(size_t offset);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] offset
 * \param  [in] state
 * \return
 */
int
gvSetOffsetState(size_t           offset,
		 GVoffsetstateptr state);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
int
gvDelThreadTransport(void);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
GVtransportptr
gvGetThreadTransport(void);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \return
 */
int
gvSetThreadTransport(GVtransportptr transport);

#endif /* SERVER_STATE_TRACKER_0_H_ */
