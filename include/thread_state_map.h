/*! ***************************************************************************
 * \file    thread_state_map.h
 * \brief
 * 
 * \date    January 9, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef THREAD_STATE_MAP_H_
#define THREAD_STATE_MAP_H_

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
int
gvInitThreadStateMap(void);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] key
 * \return
 */
int
gvDelThreadItem(unsigned long key);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] key
 * \return
 */
void
*gvGetThreadItem(unsigned long int key);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] key
 * \param  [in] value
 * \return
 */
int
gvPutThreadItem(unsigned long int  key,
		void              *value);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
int
gvTermThreadStateMap();

#endif /* THREAD_STATE_MAP_H_*/
