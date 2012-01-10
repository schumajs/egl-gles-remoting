/*! ***************************************************************************
 * \file    memory_manager.h
 * \brief   
 * 
 * \date    December 14, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

/*! ***************************************************************************
 * \namespace gvmmgr
 */

#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <unistd.h>

/*! ***************************************************************************
 * \brief
 *
 * \param [out] offset
 * \param [in]  length
 * \return
 */
int gvmmgrAlloc(size_t *offset, size_t length);

/*! ***************************************************************************
 * \brief
 *
 * \param [out] data
 * \return
 */
int gvmmgrFree(size_t offset);

#endif /* MEMORY_MANAGER_H */
