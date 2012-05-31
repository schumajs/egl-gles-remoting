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

typedef void (*GVpstateiterfunc)(unsigned long int, void*, void*);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] key
 * \return
 */
int
gvDelProcessStateItem(unsigned long int key);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in]  key
 * \return
 */
void
*gvGetProcessStateItem(unsigned long int key);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] func
 * \param  [in] arg
 * \return
 */
int
gvForeachProcessStateItem(GVpstateiterfunc  func,
			  void             *arg);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] key
 * \param  [in] value
 * \return
 */
int
gvPutProcessStateItem(unsigned long int  key,
		      void              *value);

#endif /* PROCESS_STATE_MAP_H_*/
