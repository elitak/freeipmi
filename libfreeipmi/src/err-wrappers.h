/* 
   err-wrappers.h - IPMI error handling

   Copyright (C) 2003, 2004, 2005 FreeIPMI Core Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  

*/


#ifndef _ERR_WRAPPERS_H
#define	_ERR_WRAPPERS_H

#ifdef __cplusplus
extern "C" {
#endif

  /* XXX need to do includes */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define ERR_WRAPPER_STR_MAX_LEN 4096

#if defined (IPMI_SYSLOG)
#define __IPMI_SYSLOG                                                   \
      char errstr[ERR_WRAPPER_STR_MAX_LEN];                             \
      snprintf (errstr, ERR_WRAPPER_STR_MAX_LEN,                        \
               "%s: %d: %s: errno (%d): expression failed", __FILE__,   \
               __LINE__, __PRETTY_FUNCTION__, save_errno);              \
      syslog (LOG_MAKEPRI (LOG_FAC (LOG_LOCAL1), LOG_ERR), errstr);
#else
#define __IPMI_SYSLOG
#endif /* IPMI_SYSLOG */

#if defined (IPMI_TRACE)
#define __IPMI_TRACE                                                    \
      fprintf (stderr,                                                  \
               "%s: %d: %s: errno (%d): expression failed\n", __FILE__, \
               __LINE__, __PRETTY_FUNCTION__, save_errno);              \
      fflush (stderr);                                                  
#else
#define __IPMI_TRACE
#endif /* IPMI_TRACE */

#define ERR(expr)                                                       \
do {                                                                    \
  if (!(expr))                                                          \
    {                                                                   \
      extern int errno;                                                 \
      int save_errno = errno;                                           \
      __IPMI_SYSLOG;                                                    \
      __IPMI_TRACE;                                                     \
      errno = save_errno;                                               \
      return (-1);                                                      \
    }                                                                   \
} while (0)

#define ERR_CLEANUP(expr)                                               \
do {                                                                    \
  if (!(expr))                                                          \
    {                                                                   \
      extern int errno;                                                 \
      int save_errno = errno;                                           \
      __IPMI_SYSLOG;                                                    \
      __IPMI_TRACE;                                                     \
      errno = save_errno;                                               \
      goto cleanup;                                                     \
    }                                                                   \
} while (0)

#define ERR_EXIT(expr)                                                  \
do {                                                                    \
  if (!(expr))                                                          \
    {                                                                   \
      extern int errno;                                                 \
      int save_errno = errno;                                           \
      __IPMI_SYSLOG;                                                    \
      __IPMI_TRACE;                                                     \
      errno = save_errno;                                               \
      exit(1);                                                          \
    }                                                                   \
} while (0)

#define ERR_IPMI_CMD_CLEANUP(__dev, __lun, __netfn, __rq, __rs)         \
do {                                                                    \
  ERR_CLEANUP (!(ipmi_cmd ((__dev),                                     \
                           (__lun),                                     \
                           (__netfn),                                   \
                           (__rq),                                      \
                           (__rs)) < 0));                               \
  ERR_CLEANUP (ipmi_comp_test ((__rs)) == 1);                           \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* err-wrappers.h */

