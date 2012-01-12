/*! ***************************************************************************
 * \file    server_serializer.h
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
GVcallid gvCall();

/*! ***************************************************************************
 * \brief
 *
 * \param [in] data
 * \param [in] length
 */
void gvGetData(void *data, size_t length);

/*! ***************************************************************************
 * \brief
 */
void gvEndCall();

/*! ***************************************************************************
 * \brief
 *
 * \param [in] callId
 */
void gvReturn(GVcallid callId);

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
void gvEndReturn();

#endif /* SERVER_SERIALIZER_H_ */
