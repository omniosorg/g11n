/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").  
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*[]---------------------------------------------------[]*/
/*|                                                     |*/
/*|     CTL Threads definitions                         |*/
/*|     ctl_threads.h                                   |*/
/*|                                                     |*/
/*|     Copyright (c) 1996 by Sun Microsystems, Inc.    |*/
/*|     All Rights Reserved.                            |*/
/*|                                                     |*/
/*[]---------------------------------------------------[]*/
 
#ident "@(#)ctl_threads.h 1.2 96/09/19 SMI"

#ifndef __CTL_THREADS_H
#define __CTL_THREADS_H

#ifdef  __cplusplus
extern "C" {
#endif
#ifdef CTL_THREADS 

  #include <pthread.h>
  #define ctl_mutex_t pthread_mutex_t
  #define ctl_mutexattr_t pthread_mutexattr_t
  #define ctl_mutexattr_setpshared(mattr, param) pthread_mutexattr_setpshared(mattr, param)
  #define ctl_mutex_init(mutex, attr) pthread_mutex_init(mutex, attr)
  #define ctl_mutex_lock(mutex) pthread_mutex_lock(mutex)
  #define ctl_mutex_unlock(mutex) pthread_mutex_unlock(mutex)

#else

  #define ctl_mutex_t int
  #define ctl_mutexattr_t int
  #define ctl_mutexattr_setpshared(mattr, param)
  #define ctl_mutex_init(mutex, attr)
  #define ctl_mutex_lock(mutex) 
  #define ctl_mutex_unlock(mutex) 

#endif /* CTL_THREADS */

#ifdef  __cplusplus
}
#endif



#endif /* __CTL_THREADS_H */

