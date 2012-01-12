/*! ***************************************************************************
 * \file    server.h
 * \brief   
 * 
 * \date    January 6, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

/*! ***************************************************************************
 * \namespace gvsrv
 */

#include <unistd.h>

#ifndef SERVER_H_
#define SERVER_H_

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] offset
 * \param  [in] length
 * \return 
 */
int gvsrvBonjour(size_t offset, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
int gvsrvAuRevoir(void);

#endif /* SERVER_H_ */
