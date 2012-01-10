/*! ***************************************************************************
 * \file    serializer.h
 * \brief
 * 
 * \date    December 27, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERVER_SERIALIZER_H_
#define SERVER_SERIALIZER_H_

#include "../serializer.h"

/*! ***************************************************************************
 * \brief 
 *
 * \return
 */
GVSERcallid gvserCall();

/*! ***************************************************************************
 * \brief
 *
 * \param [in] data
 * \param [in] length
 */
void gvserInParameter(void *data, size_t length);

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
 * \param [in] data
 * \param [in] length
 */
void gvserReturnValue(const void *data, size_t length);

/*! ***************************************************************************
 * \brief
 *
 * \param [in] data
 * \param [in] length
 */
void gvserOutParameter(const void *data, size_t length);

/*! ***************************************************************************
 * \brief
 */
void gvserEndReturn();

#endif /* SERVER_SERIALIZER_H_ */
