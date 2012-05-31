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

typedef void (*GVtstateiterfunc)(unsigned long int, void*, void*);

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
gvDelThreadStateItem(unsigned long key);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] key
 * \return
 */
void
*gvGetThreadStateItem(unsigned long int key);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] func
 * \param  [in] arg
 * \return
 */
int
gvForeachThreadStateItem(GVtstateiterfunc  func,
			 void             *arg);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] key
 * \param  [in] value
 * \return
 */
int
gvPutThreadStateItem(unsigned long int  key,
		     void              *value);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
int
gvTermThreadStateMap();

#endif /* THREAD_STATE_MAP_H_*/
