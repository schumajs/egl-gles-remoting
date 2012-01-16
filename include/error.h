/*! ***************************************************************************
 * \file    globals.0.h
 * \brief
 * 
 * \date    January 15, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

/* ***************************************************************************
 * Syntactic suggar for error handling
 */

#define TRY(lbl)
#define THROW(to, msg) perror(msg); goto to
#define CATCH(to) while (0) to:
