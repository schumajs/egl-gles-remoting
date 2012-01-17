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
 * \param  [in] keyPtr
 * \param  [in] keyLength
 * \return
 */
int
gvDelProcessState(void   *keyPtr,
                  size_t  keyLength);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] keyPtr
 * \param  [in] keyLength
 * \param  [in] valPtr
 * \return
 */
int
gvGetProcessState(void   *keyPtr,
                  size_t  keyLength,
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
gvPutProcessState(void   *keyPtr,
                  size_t  keyLength,
		  void   *valPtr);

#endif /* PROCESS_STATE_MAP_H_*/
