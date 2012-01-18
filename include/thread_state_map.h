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
gvDelThreadState(unsigned long key);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in]  key
 * \param  [out] value
 * \return
 */
int
gvGetThreadState(unsigned long int   key,
		 void              **value);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] key
 * \param  [in] value
 * \return
 */
int
gvPutThreadState(unsigned long int  key,
		 void              *value);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
int
gvTermThreadStateMap();

#endif /* THREAD_STATE_MAP_H_*/
