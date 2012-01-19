/*! ***************************************************************************
 * \file    heap_manager.h
 * \brief   
 * 
 * \date    December 14, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef HEAP_MANAGER_H_
#define HEAP_MANAGER_H_

#include <unistd.h>

/*! ***************************************************************************
 * \brief
 *
 * \param [in]  length
 * \return
 */
size_t
gvAlloc(size_t length);

/*! ***************************************************************************
 * \brief
 *
 * \param [out] data
 * \return
 */
int
gvFree(size_t offset);

#endif /* HEAP_MANAGER_H */
