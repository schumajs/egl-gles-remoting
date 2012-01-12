/*! ***************************************************************************
 * \file    janitor.h
 * \brief   
 * 
 * \date    January 6, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <unistd.h>

#ifndef JANITOR_H_
#define JANITOR_H_

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] offset
 * \param  [in] length
 * \return 
 */
int gvBonjour(size_t offset, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
int gvAuRevoir(void);

#endif /* JANITOR_H_ */
