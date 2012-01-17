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
 * \param  [in] keyPtr
 * \param  [in] keyLength
 * \param  [in] valPtr
 * \return
 */
int
gvDelThreadState(void   *keyPtr,
                 size_t  keyLength);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in]  keyPtr
 * \param  [in]  keyLength
 * \param  [out] valPtr
 * \return
 */
int
gvGetThreadState(void    *keyPtr,
                 size_t   keyLength,
		 void   **valPtr);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] keyPtr
 * \param  [in] keyLength
 * \param  [in] valPtr
 * \return
 */
int
gvPutThreadState(void   *keyPtr,
                 size_t  keyLength,
		 void   *valPtr);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
int
gvTermThreadStateMap();

#endif /* THREAD_STATE_MAP_H_*/
