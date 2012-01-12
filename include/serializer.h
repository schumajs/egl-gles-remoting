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

#define GVSER_SRV_BONJOUR                        0
#define GVSER_SRV_AUREVOIR                       1

#define GVSER_RESERVED_0                         0
#define GVSER_RESERVED_1                         1
#define GVSER_RESERVED_2                         2
#define GVSER_RESERVED_3                         3
#define GVSER_RESERVED_4                         4
#define GVSER_RESERVED_5                         5
#define GVSER_RESERVED_6                         6
#define GVSER_RESERVED_7                         7
#define GVSER_RESERVED_8                         8
#define GVSER_RESERVED_9                         9

#define GVSER_EGL_GETERROR                      10
#define GVSER_EGL_GETDISPLAY                    11
#define GVSER_EGL_INITIALIZE                    12
#define GVSER_EGL_TERMINATE                     13
#define GVSER_EGL_QUERYSTRING                   14
#define GVSER_EGL_GETCONFIGS                    15
#define GVSER_EGL_CHOOSECONFIG                  16
#define GVSER_EGL_GETCONFIGATTRIB               17
#define GVSER_EGL_CREATEWINDOWSURFACE           18
#define GVSER_EGL_CREATEPBUFFERSURFACE          19
#define GVSER_EGL_CREATEPIXMAPSURFACE           20
#define GVSER_EGL_DESTROYSURFACE                21
#define GVSER_EGL_QUERYSURFACE                  22
#define GVSER_EGL_BINDAPI                       23
#define GVSER_EGL_QUERYAPI                      24
#define GVSER_EGL_WAITCLIENT                    25
#define GVSER_EGL_RELEASETHREAD                 26
#define GVSER_EGL_CREATEPBUFFERFROMCLIENTBUFFER 27
#define GVSER_EGL_SURFACEATTRIB                 28
#define GVSER_EGL_BINDTEXIMAGE                  29
#define GVSER_EGL_RELEASETEXIMAGE               30
#define GVSER_EGL_SWAPINTERVAL                  31
#define GVSER_EGL_CREATECONTEXT                 32
#define GVSER_EGL_DESTROYCONTEXT                33
#define GVSER_EGL_MAKECURRENT                   34
#define GVSER_EGL_GETCURRENTCONTEXT             35
#define GVSER_EGL_GETCURRENTSURFACE             36
#define GVSER_EGL_GETCURRENTDISPLAY             37
#define GVSER_EGL_QUERYCONTEXT                  38
#define GVSER_EGL_WAITGL                        39
#define GVSER_EGL_WAITNATIVE                    40
#define GVSER_EGL_SWAPBUFFERS                   41
#define GVSER_EGL_COPYBUFFERS                   42

#endif /* SERIALIZER_H */
