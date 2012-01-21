/*! ***************************************************************************
 * \file    serializer.h
 * \brief
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERIALZIER_H
#define SERIALIZER_H

#include "lock.h"
#include "transport.h"

typedef int GVcmdid;
typedef int GVcallid;

#define GV_CMDID_HMGR_ALLOC                         0x0
#define GV_CMDID_HMGR_FREE                          0x1

#define GV_CMDID_JANITOR_BONJOUR                    0x0
#define GV_CMDID_JANITOR_AUREVOIR                   0x1

#define GV_CMDID_RESERVED_0                        0x00
#define GV_CMDID_RESERVED_1                        0x01
#define GV_CMDID_RESERVED_2                        0x02
#define GV_CMDID_RESERVED_3                        0x03
#define GV_CMDID_RESERVED_4                        0x04
#define GV_CMDID_RESERVED_5                        0x05
#define GV_CMDID_RESERVED_6                        0x06
#define GV_CMDID_RESERVED_7                        0x07
#define GV_CMDID_RESERVED_8                        0x08
#define GV_CMDID_RESERVED_9                        0x09

#define GV_CMDID_EGL_GETERROR                      0x0a
#define GV_CMDID_EGL_GETDISPLAY                    0x0b
#define GV_CMDID_EGL_INITIALIZE                    0x0c
#define GV_CMDID_EGL_TERMINATE                     0x0d
#define GV_CMDID_EGL_QUERYSTRING                   0x0e
#define GV_CMDID_EGL_GETCONFIGS                    0x0f
#define GV_CMDID_EGL_CHOOSECONFIG                  0x10
#define GV_CMDID_EGL_GETCONFIGATTRIB               0x11
#define GV_CMDID_EGL_CREATEWINDOWSURFACE           0x12
#define GV_CMDID_EGL_CREATEPBUFFERSURFACE          0x13
#define GV_CMDID_EGL_CREATEPIXMAPSURFACE           0x14
#define GV_CMDID_EGL_DESTROYSURFACE                0x15
#define GV_CMDID_EGL_QUERYSURFACE                  0x16
#define GV_CMDID_EGL_BINDAPI                       0x17
#define GV_CMDID_EGL_QUERYAPI                      0x18
#define GV_CMDID_EGL_WAITCLIENT                    0x19
#define GV_CMDID_EGL_RELEASETHREAD                 0x1a
#define GV_CMDID_EGL_CREATEPBUFFERFROMCLIENTBUFFER 0x1b
#define GV_CMDID_EGL_SURFACEATTRIB                 0x1c
#define GV_CMDID_EGL_BINDTEXIMAGE                  0x1d
#define GV_CMDID_EGL_RELEASETEXIMAGE               0x1e
#define GV_CMDID_EGL_SWAPINTERVAL                  0x1f
#define GV_CMDID_EGL_CREATECONTEXT                 0x20
#define GV_CMDID_EGL_DESTROYCONTEXT                0x21
#define GV_CMDID_EGL_MAKECURRENT                   0x22
#define GV_CMDID_EGL_GETCURRENTCONTEXT             0x23
#define GV_CMDID_EGL_GETCURRENTSURFACE             0x24
#define GV_CMDID_EGL_GETCURRENTDISPLAY             0x25
#define GV_CMDID_EGL_QUERYCONTEXT                  0x26
#define GV_CMDID_EGL_WAITGL                        0x27
#define GV_CMDID_EGL_WAITNATIVE                    0x28
#define GV_CMDID_EGL_SWAPBUFFERS                   0x29
#define GV_CMDID_EGL_COPYBUFFERS                   0x2a

#define GV_CMDID_GLES2_CREATESHADER                0x2b
#define GV_CMDID_GLES2_SHADERSOURCE                0x2c
#define GV_CMDID_GLES2_COMPILESHADER               0x2d
#define GV_CMDID_GLES2_GETSHADERIV                 0x2e
#define GV_CMDID_GLES2_GETSHADERINFOLOG            0x2f
#define GV_CMDID_GLES2_DELETESHADER                0x30
#define GV_CMDID_GLES2_CREATEPROGRAM               0x31

/*! ***************************************************************************
 * \brief
 * 
 * \param  [in] transport
 * \param  [in] toAddr
 * \param  [in] length
 * \return
 */
int
gvReceiveData(GVtransportptr  transport,
	      void           *toAddr, 
	      size_t          length);

/*! ***************************************************************************
 * \brief
 * 
 * \param  [in] transport
 * \return
 */
void
*gvReceiveVarSizeData(GVtransportptr transport);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \param  [in] fromAddr
 * \param  [in] length
 * \return
 */
int
gvSendData(GVtransportptr  transport,
	   const void     *fromAddr,
	   size_t          length);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \param  [in] fromAddr
 * \param  [in] length
 * \return
 */
int
gvSendVarSizeData(GVtransportptr  transport,
		  const void     *fromAddr,
		  size_t          length);

#endif /* SERIALIZER_H */
