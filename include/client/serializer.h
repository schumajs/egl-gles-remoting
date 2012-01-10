/*! ***************************************************************************
 * \file    serializer.h
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
GVSERcallid gvserCall(GVSERcmdid cmdId);

/*! ***************************************************************************
 * \brief
 *
 * \param [in] data
 * \param [in] length
 */
void gvserInParameter(const void *data, size_t length);

/*! ***************************************************************************
 * \brief
 */
void gvserEndCall();

/*! ***************************************************************************
 * \brief
 *
 * \param [in] callId
 */
void gvserReturn(GVSERcallid callId);

/*! ***************************************************************************
 * \brief
 *
 * \param [out] data
 * \param [in]  length
 */
void gvserReturnValue(void *data, size_t length);

/*! ***************************************************************************
 * \brief
 *
 * \param [out] data
 * \param [in]  length
 */
void gvserOutParameter(void *data, size_t length);

/*! ***************************************************************************
 * \brief
 */
void gvserEndReturn();

#endif /* CLIENT_SERIALIZER_H_ */
