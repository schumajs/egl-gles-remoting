/*! ***************************************************************************
 * \file    error.h
 * \brief
 * 
 * \date    January 15, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>

/* ***************************************************************************
 * Syntactic suggar for error handling
 */

#define TRY
#define THROW(to, msg) perror(msg); goto to
#define CATCH(to) while (0) to:

#endif /* ERROR_H_ */
