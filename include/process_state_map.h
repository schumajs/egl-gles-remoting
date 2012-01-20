/*! ***************************************************************************
 * \file    process_state_map.h
 * \brief
 * 
 * \date    January 9, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef PROCESS_STATE_MAP_H_
#define PROCESS_STATE_MAP_H_

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] key
 * \return
 */
int
gvDelProcessItem(unsigned long int key);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in]  key
 * \return
 */
void
*gvGetProcessItem(unsigned long int key);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] key
 * \param  [in] value
 * \return
 */
int
gvPutProcessItem(unsigned long int  key,
		 void              *value);

#endif /* PROCESS_STATE_MAP_H_*/
