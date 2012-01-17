/*! ***************************************************************************
 * \file    janitor.h
 * \brief   
 * 
 * \date    January 6, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef JANITOR_H_
#define JANITOR_H_

#include <unistd.h>

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] transportOffset
 * \param  [in] tansportLength
 * \return 
 */
int
gvBonjour(size_t offset,
	  size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] transportOffset
 * \return 
 */
int
gvAuRevoir(size_t offset);

#endif /* JANITOR_H_ */
