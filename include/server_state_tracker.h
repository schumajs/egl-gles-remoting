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
#include <GLES2/gl2.h>

#include "shm_stream_transport.h"

struct GVoffsetstate {
    unsigned long     thread;
    GVshmstreamtrpptr transport;
};

typedef struct GVoffsetstate *GVoffsetstateptr;

struct GVvertexattrib {
    GLuint        index;
    GLint         size;
    GLenum        type;
    GLboolean     normalized;
    GLsizei       stride;
};

typedef struct GVvertexattrib *GVvertexattribptr;

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
gvPutOffsetState(size_t           offset,
		 GVoffsetstateptr state);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
GVtransportptr
gvGetCurrentThreadTransport(void);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \return
 */
int
gvSetCurrentThreadTransport(GVtransportptr transport);

#endif /* SERVER_STATE_TRACKER_0_H_ */
