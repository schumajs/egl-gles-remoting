/*! ***************************************************************************
 * \file    client_serializer.h
 * \brief
 * 
 * \date    December 13, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef CLIENT_SERIALZIER_H_
#define CLIENT_SERIALIZER_H

#include "../serializer.h"

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] cmdId
 * \return
 */
GVcallid gvCall(GVcmdid cmdId);

/*! ***************************************************************************
 * \brief
 *
 * \param [in] data
 * \param [in] length
 */
void gvPutData(const void *data, size_t length);

/*! ***************************************************************************
 * \brief
 */
void gvEndCall();

/*! ***************************************************************************
 * \brief
 *
 * \param [in] callId
 */
void gvReturn(GVSERcallid callId);

/*! ***************************************************************************
 * \brief
 *
 * \param [out] data
 * \param [in]  length
 */
void gvGetData(void *data, size_t length);

/*! ***************************************************************************
 * \brief
 */
void gvEndReturn();

#endif /* CLIENT_SERIALIZER_H_ */
