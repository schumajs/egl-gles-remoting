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

/*! ***************************************************************************
 * \namespace gvser
 */

#ifndef SERIALZIER_H
#define SERIALIZER_H

#include <stdint.h>
#include <stdlib.h>

typedef int GVSERcmdid;
typedef int GVSERcallid;

#define GVSER_MMGR_ALLOC                         0
#define GVSER_MMGR_FREE                          1

#define GVSER_SRV_HANDSHAKE                      0

#define GVSER_EGL_GETERROR                       0
#define GVSER_EGL_GETDISPLAY                     1
#define GVSER_EGL_INITIALIZE                     2
#define GVSER_EGL_TERMINATE                      3
#define GVSER_EGL_QUERYSTRING                    4
#define GVSER_EGL_GETCONFIGS                     5
#define GVSER_EGL_CHOOSECONFIG                   6
#define GVSER_EGL_GETCONFIGATTRIB                7
#define GVSER_EGL_CREATEWINDOWSURFACE            8
#define GVSER_EGL_CREATEPBUFFERSURFACE           9
#define GVSER_EGL_CREATEPIXMAPSURFACE           10
#define GVSER_EGL_DESTROYSURFACE                11
#define GVSER_EGL_QUERYSURFACE                  12
#define GVSER_EGL_BINDAPI                       13
#define GVSER_EGL_QUERYAPI                      14
#define GVSER_EGL_WAITCLIENT                    15
#define GVSER_EGL_RELEASETHREAD                 16
#define GVSER_EGL_CREATEPBUFFERFROMCLIENTBUFFER 17
#define GVSER_EGL_SURFACEATTRIB                 18
#define GVSER_EGL_BINDTEXIMAGE                  19
#define GVSER_EGL_RELEASETEXIMAGE               20
#define GVSER_EGL_SWAPINTERVAL                  21
#define GVSER_EGL_CREATECONTEXT                 22
#define GVSER_EGL_DESTROYCONTEXT                23
#define GVSER_EGL_MAKECURRENT                   24
#define GVSER_EGL_GETCURRENTCONTEXT             25
#define GVSER_EGL_GETCURRENTSURFACE             26
#define GVSER_EGL_GETCURRENTDISPLAY             27
#define GVSER_EGL_QUERYCONTEXT                  28
#define GVSER_EGL_WAITGL                        29
#define GVSER_EGL_WAITNATIVE                    30
#define GVSER_EGL_SWAPBUFFERS                   31
#define GVSER_EGL_COPYBUFFERS                   32

#endif /* SERIALIZER_H */
