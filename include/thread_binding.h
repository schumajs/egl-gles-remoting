/*! ***************************************************************************
 * \file    thread_binding.h
 * \brief   
 * 
 * \date    January 12, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef THREAD_BINDING_H_
#define THREAD_BINDING_H_

struct GVbinding {
    void *data;
};

typedef struct GVbinding *GVbindingptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [out] newBinding
 * \param  [in]  data
 * \return 
 */
int gvCreateBinding(GVbindingptr *newBinding, void *data);

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
int gvBind(GVbindingptr binding);

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
GVbindingptr gvGetBinding(void);

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
int gvUnbind(GVbindingptr binding);

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
int gvDestroyBinding(GVbindingptr binding);

#endif /* THREAD_BINDING_H_ */
