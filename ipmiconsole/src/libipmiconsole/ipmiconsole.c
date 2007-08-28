/*****************************************************************************\
 *  $Id: ipmiconsole.c,v 1.61 2007-08-28 17:50:08 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2006 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>
 *  UCRL-CODE-221226
 *  
 *  This file is part of Ipmiconsole, a set of IPMI 2.0 SOL libraries
 *  and utilities.  For details, see http://www.llnl.gov/linux/.
 *  
 *  Ipmiconsole is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by the 
 *  Free Software Foundation; either version 2 of the License, or (at your 
 *  option) any later version.
 *  
 *  Ipmiconsole is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 *  for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with Ipmiconsole; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
\*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else /* !HAVE_SYS_TIME_H */
#include <time.h>
#endif  /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#include <sys/types.h>
#include <sys/select.h>
#ifdef WITH_PTHREADS
#include <pthread.h>
#endif /* WITH_PTHREADS */
#include <sys/resource.h>
#include <assert.h>
#include <errno.h>

#include "ipmiconsole.h"
#include "ipmiconsole_defs.h"

#include "secure.h"
#include "ipmiconsole_debug.h"
#include "ipmiconsole_engine.h"
#include "ipmiconsole_fiid_wrappers.h"
#include "ipmiconsole_util.h"

/* 
 * ipmi console errmsgs
 */
static char *ipmiconsole_errmsgs[] =
  {
    "success",			                        /* 0 */
    "ctx null",		                                /* 1 */
    "ctt invalid",		                        /* 2 */
    "engine already setup",	                        /* 3 */
    "engine not setup",		                        /* 4 */
    "ctx not submitted",	                        /* 5 */
    "ctx is submitted",	                                /* 6 */
    "invalid parameters",	                        /* 7 */
    "hostname invalid",		                        /* 8 */
    "ipmi 2.0 unavailable",	                        /* 9 */
    "cipher suite id unavailable",                      /* 10 */
    "username invalid",		                        /* 11 */
    "password invalid",		                        /* 12 */
    "k_g invalid",		                        /* 13 */
    "privilege level insufficient",                     /* 14 */
    "privilege level cannot be obtained for this user", /* 15 */
    "SOL unavailable",		                        /* 16 */
    "SOL in use",		                        /* 17 */
    "SOL session stolen",                               /* 18 */
    "SOL requires encryption",                          /* 19 */
    "SOL requires no encryption",                       /* 20 */
    "BMC Busy",			                        /* 21 */
    "BMC Error",		                        /* 22 */
    "BMC Implementation",                               /* 23 */
    "session timeout",		                        /* 24 */
    "excess retransmissions sent",                      /* 25 */
    "excess errors received",                           /* 26 */
    "out of memory",		                        /* 27 */
    "too many open files",                              /* 28 */
    "internal system error",	                        /* 20 */
    "internal error",		                        /* 30 */
    "errnum out of range",	                        /* 31 */
    NULL
  };

static void
_ipmiconsole_ctx_fds_init(ipmiconsole_ctx_t c)
{
  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(c->api_magic = IPMICONSOLE_CTX_API_MAGIC);

  /* init to -1 b/c -1 isn't a legit fd */
  c->fds.user_fd = -1;
  c->fds.asynccomm[0] = -1;
  c->fds.asynccomm[1] = -1;
}

static void
_ipmiconsole_ctx_fds_cleanup(ipmiconsole_ctx_t c)
{
  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(c->api_magic = IPMICONSOLE_CTX_API_MAGIC);

  /* Note: Close asynccomm[0] first, so an EBADFD error occurs in the
   * engine.  Closing asynccomm[1] first could result in a EPIPE
   * instead.
   */
  close(c->fds.user_fd);
  close(c->fds.asynccomm[0]);
  close(c->fds.asynccomm[1]);
}

static int
_ipmiconsole_ctx_signal_init(ipmiconsole_ctx_t c)
{
  int perr;

  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(c->api_magic = IPMICONSOLE_CTX_API_MAGIC);

  if ((perr = pthread_mutex_init(&c->signal.status_mutex, NULL)) != 0)
    {
      errno = perr;
      return -1;
    }
  c->signal.status = IPMICONSOLE_CTX_STATUS_NOT_SUBMITTED;

  if ((perr = pthread_mutex_init(&c->signal.destroyed_mutex, NULL)) != 0)
    {
      errno = perr;
      return -1;
    }
  c->signal.user_has_destroyed = 0;
  c->signal.moved_to_destroyed = 0;

  return 0;
}

static void
_ipmiconsole_ctx_signal_cleanup(ipmiconsole_ctx_t c)
{
  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(c->api_magic = IPMICONSOLE_CTX_API_MAGIC);

  pthread_mutex_destroy(&(c->signal.status_mutex));
  pthread_mutex_destroy(&(c->signal.destroyed_mutex));
}

static int
_ipmiconsole_ctx_blocking_init(ipmiconsole_ctx_t c)
{
  int perr;

  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(c->api_magic = IPMICONSOLE_CTX_API_MAGIC);

  if ((perr = pthread_mutex_init(&c->blocking.blocking_mutex, NULL)) != 0)
    {
      errno = perr;
      return -1;
    }
  c->blocking.blocking_submit_requested = 0;
  c->blocking.blocking_notification[0] = -1;
  c->blocking.blocking_notification[1] = -1;
  c->blocking.sol_session_established = 0;

  return 0;
}

static void
_ipmiconsole_ctx_blocking_cleanup(ipmiconsole_ctx_t c)
{
  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(c->api_magic = IPMICONSOLE_CTX_API_MAGIC);

  pthread_mutex_destroy(&(c->blocking.blocking_mutex));
}

int 
ipmiconsole_engine_init(unsigned int thread_count, unsigned int debug_flags)
{
  struct rlimit rlim;
  int i;

  if (!thread_count
      || thread_count > IPMICONSOLE_THREAD_COUNT_MAX
      || (debug_flags & ~IPMICONSOLE_DEBUG_MASK))
    {
      errno = EINVAL;
      return -1;
    }

  /* Note: Must be called first before anything else for debugging purposes */
  if (ipmiconsole_debug_setup(debug_flags) < 0)
    goto cleanup;

  if (ipmiconsole_engine_is_setup())
    return 0;

  if (ipmiconsole_engine_setup(thread_count) < 0)
    goto cleanup;

  for (i = 0; i < thread_count; i++)
    {
      if (ipmiconsole_engine_thread_create() < 0)
        goto cleanup;
    }
 
  /* If the file descriptor increase fails, ignore it */

  if (getrlimit(RLIMIT_NOFILE, &rlim) == 0)
    {
      rlim.rlim_cur = rlim.rlim_max;
      setrlimit(RLIMIT_NOFILE, &rlim);
    }

  return 0;

 cleanup:
  ipmiconsole_debug_cleanup();
  ipmiconsole_engine_cleanup(0);
  return -1;
}

int 
ipmiconsole_engine_submit(ipmiconsole_ctx_t c)
{
  int perr;

  if (!c 
      || c->magic != IPMICONSOLE_CTX_MAGIC
      || c->api_magic != IPMICONSOLE_CTX_API_MAGIC)
    return -1;

  if (!ipmiconsole_engine_is_setup())
    {
      c->errnum = IPMICONSOLE_ERR_NOT_SETUP;
      return -1;
    }

  if (c->session_submitted)
    {
      c->errnum = IPMICONSOLE_ERR_CTX_IS_SUBMITTED;
      return -1;
    }

  if (_ipmiconsole_ctx_connection_setup(c) < 0)
    goto cleanup;

  if (_ipmiconsole_ctx_session_init(c) < 0)
    goto cleanup;
  
  if (ipmiconsole_engine_submit_ctx(c) < 0)
    goto cleanup;

  if ((perr = pthread_mutex_lock(&(c->signal.status_mutex))) != 0)
    {
      IPMICONSOLE_DEBUG(("pthread_mutex_lock: %s", strerror(perr)));
      c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
      goto cleanup;
    }
  
  /* Check for NONE status, conceivably ERROR or SOL_ESTABLISHED could
   * already be set 
   */
  if (c->signal.status == IPMICONSOLE_CTX_STATUS_NOT_SUBMITTED)
    c->signal.status = IPMICONSOLE_CTX_STATUS_SUBMITTED;
  
  if ((perr = pthread_mutex_unlock(&(c->signal.status_mutex))) != 0)
    {
      IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(perr)));
      c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
      goto cleanup;
    }

  c->session_submitted++;
  return 0;

 cleanup:
  _ipmiconsole_ctx_connection_cleanup(c);
  _ipmiconsole_ctx_fds_cleanup(c);
  _ipmiconsole_ctx_fds_init(c);
  return -1;
}

static int
_ipmiconsole_blocking_notification_cleanup(ipmiconsole_ctx_t c)
{
  int perr;

  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(c->api_magic == IPMICONSOLE_CTX_API_MAGIC);

  if (c->blocking.blocking_submit_requested)
    {
      if ((perr = pthread_mutex_lock(&(c->blocking.blocking_mutex))) != 0)
        {
          IPMICONSOLE_DEBUG(("pthread_mutex_lock: %s", strerror(perr)));
          c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
          return -1;
        }

      close(c->blocking.blocking_notification[0]);
      close(c->blocking.blocking_notification[1]);
      c->blocking.blocking_notification[0] = -1;
      c->blocking.blocking_notification[1] = -1;
      c->blocking.blocking_submit_requested = 0;
      c->blocking.sol_session_established = 0;
      
      if ((perr = pthread_mutex_unlock(&(c->blocking.blocking_mutex))) != 0)
        {
          IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(perr)));
          c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
          return -1;
        }
    }

  return 0;
}

static int
_ipmiconsole_blocking_notification_setup(ipmiconsole_ctx_t c)
{
  int perr;

  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(c->api_magic == IPMICONSOLE_CTX_API_MAGIC);
  
  if ((perr = pthread_mutex_lock(&(c->blocking.blocking_mutex))) != 0)
    {
      IPMICONSOLE_DEBUG(("pthread_mutex_lock: %s", strerror(perr)));
      c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
      goto cleanup;
    }

  if (pipe(c->blocking.blocking_notification) < 0)
    {
      if ((perr = pthread_mutex_unlock(&(c->blocking.blocking_mutex))) != 0)
        IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(perr)));
      IPMICONSOLE_CTX_DEBUG(c, ("pipe: %s", strerror(errno)));
      if (errno == EMFILE)
        c->errnum = IPMICONSOLE_ERR_TOO_MANY_OPEN_FILES;
      else
        c->errnum = IPMICONSOLE_ERR_SYSTEM_ERROR;
      goto cleanup;
    }
  c->blocking.blocking_submit_requested++;
  c->blocking.sol_session_established = 0;

  if ((perr = pthread_mutex_unlock(&(c->blocking.blocking_mutex))) != 0)
    {
      IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(perr)));
      c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
      goto cleanup;
    }

  if (ipmiconsole_set_closeonexec(c, c->blocking.blocking_notification[0]) < 0)
    {
      IPMICONSOLE_DEBUG(("closeonexec error"));
      goto cleanup;
    }
  if (ipmiconsole_set_closeonexec(c, c->blocking.blocking_notification[1]) < 0)
    {
      IPMICONSOLE_DEBUG(("closeonexec error"));
      goto cleanup;
    }

  return 0;

 cleanup:
  _ipmiconsole_blocking_notification_cleanup(c);
  return -1;
}

static int
_ipmiconsole_block(ipmiconsole_ctx_t c)
{
  fd_set rds;
  int n;
  
  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(c->api_magic == IPMICONSOLE_CTX_API_MAGIC);
  assert(c->blocking.blocking_submit_requested);

  FD_ZERO(&rds);
  FD_SET(c->blocking.blocking_notification[0], &rds);

  if ((n = select(c->blocking.blocking_notification[0] + 1, &rds, NULL, NULL, NULL)) < 0)
    {
      IPMICONSOLE_CTX_DEBUG(c, ("select: %s", strerror(errno)));
      c->errnum = IPMICONSOLE_ERR_SYSTEM_ERROR;
      goto cleanup;
    }

  if (!n)
    {
      IPMICONSOLE_CTX_DEBUG(c, ("select returned 0"));
      c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
      goto cleanup;
    }

  if (FD_ISSET(c->blocking.blocking_notification[0], &rds))
    {
      uint8_t val;
      ssize_t len;

      if ((len = read(c->blocking.blocking_notification[0], (void *)&val, 1)) < 0)
        {
          IPMICONSOLE_CTX_DEBUG(c, ("read: %s", strerror(errno)));
          c->errnum = IPMICONSOLE_ERR_SYSTEM_ERROR;
          goto cleanup;
        }
      
      if (!len)
        {
          IPMICONSOLE_CTX_DEBUG(c, ("blocking_notification closed"));
          c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
          goto cleanup;
        }

      if (val == IPMICONSOLE_BLOCKING_NOTIFICATION_SOL_SESSION_ESTABLISHED)
        goto success;
      else if (val == IPMICONSOLE_BLOCKING_NOTIFICATION_SOL_SESSION_ERROR)
        goto cleanup;
      else if (c->config.security_flags & IPMICONSOLE_SECURITY_DEACTIVATE_ONLY
               && val == IPMICONSOLE_BLOCKING_NOTIFICATION_SOL_SESSION_DEACTIVATED)
        goto success;
      else
        {
          IPMICONSOLE_CTX_DEBUG(c, ("blocking_notification returned invalid data: %d", val));
          c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
          goto cleanup;
        }
    }

 success:
  return 0;

 cleanup:
  return -1;
}

int 
ipmiconsole_engine_submit_block(ipmiconsole_ctx_t c)
{
  int perr;

  if (!c 
      || c->magic != IPMICONSOLE_CTX_MAGIC
      || c->api_magic != IPMICONSOLE_CTX_API_MAGIC)
    return -1;

  if (!ipmiconsole_engine_is_setup())
    {
      c->errnum = IPMICONSOLE_ERR_NOT_SETUP;
      return -1;
    }

  if (c->session_submitted)
    {
      c->errnum = IPMICONSOLE_ERR_CTX_IS_SUBMITTED;
      return -1;
    }

  /* Set to success, so we know if an IPMI error occurred */
  c->errnum = IPMICONSOLE_ERR_SUCCESS;
 
  if (_ipmiconsole_ctx_connection_setup(c) < 0)
    goto cleanup;

  if (_ipmiconsole_ctx_session_init(c) < 0)
    goto cleanup;

  if (_ipmiconsole_blocking_notification_setup(c) < 0)
    goto cleanup;

  if (ipmiconsole_engine_submit_ctx(c) < 0)
    goto cleanup;
      
  /* sol_session_established flag maybe set in here */
  if (_ipmiconsole_block(c) < 0)
    {
      /* don't go to cleanup, b/c the engine will call
       * _ipmiconsole_ctx_connection_cleanup().
       */
      goto cleanup_ctx_fds_only;
    }
  
  if ((perr = pthread_mutex_lock(&(c->signal.status_mutex))) != 0)
    {
      IPMICONSOLE_DEBUG(("pthread_mutex_lock: %s", strerror(perr)));
      c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
      goto cleanup_ctx_fds_only;
    }
  
  /* Check for NONE status, conceivably ERROR or SOL_ESTABLISHED could
   * already be set 
   */
  if (c->signal.status == IPMICONSOLE_CTX_STATUS_NOT_SUBMITTED)
    c->signal.status = IPMICONSOLE_CTX_STATUS_SUBMITTED;
  
  if ((perr = pthread_mutex_unlock(&(c->signal.status_mutex))) != 0)
    {
      IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(perr)));
      c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
      goto cleanup_ctx_fds_only;
    }

  c->session_submitted++;
  return 0;

 cleanup:
  _ipmiconsole_ctx_connection_cleanup(c);
 cleanup_ctx_fds_only:
  _ipmiconsole_blocking_notification_cleanup(c);
  _ipmiconsole_ctx_fds_cleanup(c);
  _ipmiconsole_ctx_fds_init(c);
  return -1;
}

void
ipmiconsole_engine_teardown(int cleanup_sol_sessions)
{
  ipmiconsole_debug_cleanup();
  ipmiconsole_engine_cleanup(cleanup_sol_sessions);
}

ipmiconsole_ctx_t 
ipmiconsole_ctx_create(char *hostname,
		       struct ipmiconsole_ipmi_config *ipmi_config,
		       struct ipmiconsole_protocol_config *protocol_config)
{
  ipmiconsole_ctx_t c = NULL;

  if (!hostname
      || (hostname && strlen(hostname) > MAXHOSTNAMELEN)
      || !ipmi_config
      || !protocol_config
      || (ipmi_config->username && strlen(ipmi_config->username) > IPMI_MAX_USER_NAME_LENGTH)
      || (ipmi_config->password && strlen(ipmi_config->password) > IPMI_2_0_MAX_PASSWORD_LENGTH)
      || (ipmi_config->k_g && ipmi_config->k_g_len > IPMI_MAX_K_G_LENGTH)
      || (ipmi_config->privilege_level >= 0
	  && (ipmi_config->privilege_level != IPMICONSOLE_PRIVILEGE_USER
	      && ipmi_config->privilege_level != IPMICONSOLE_PRIVILEGE_OPERATOR
	      && ipmi_config->privilege_level != IPMICONSOLE_PRIVILEGE_ADMIN))
      || (ipmi_config->cipher_suite_id >= IPMI_CIPHER_SUITE_ID_MIN
	  && !IPMI_CIPHER_SUITE_ID_SUPPORTED(ipmi_config->cipher_suite_id))
      || (protocol_config->engine_flags & ~IPMICONSOLE_ENGINE_MASK)
      || (protocol_config->debug_flags & ~IPMICONSOLE_DEBUG_MASK)
      || (protocol_config->security_flags & ~IPMICONSOLE_SECURITY_MASK)
      || (protocol_config->workaround_flags & ~IPMICONSOLE_WORKAROUND_MASK))
    {
      errno = EINVAL;
      return NULL;
    }

  if (protocol_config->security_flags & IPMICONSOLE_SECURITY_LOCK_MEMORY)
    {
      if (!(c = (ipmiconsole_ctx_t)secure_malloc(sizeof(struct ipmiconsole_ctx))))
        {
          errno = ENOMEM;
          return NULL;
        }
    }
  else
    {
      if (!(c = (ipmiconsole_ctx_t)malloc(sizeof(struct ipmiconsole_ctx))))
        {
          errno = ENOMEM;
          return NULL;
        }
    }

  _ipmiconsole_ctx_init(c);

  strcpy(c->config.hostname, hostname);

  if (ipmi_config->username)
    strcpy((char *)c->config.username, ipmi_config->username);

  if (ipmi_config->password)
    strcpy((char *)c->config.password, ipmi_config->password);

  /* k_g may contain nulls */
  if (ipmi_config->k_g && ipmi_config->k_g_len) 
    {
      memcpy(c->config.k_g, ipmi_config->k_g, ipmi_config->k_g_len);
      c->config.k_g_len = ipmi_config->k_g_len;
    }

  if (ipmi_config->privilege_level >= 0)
    {
      if (ipmi_config->privilege_level == IPMICONSOLE_PRIVILEGE_USER)
        c->config.privilege_level = IPMI_PRIVILEGE_LEVEL_USER;
      else if (ipmi_config->privilege_level == IPMICONSOLE_PRIVILEGE_OPERATOR)
        c->config.privilege_level = IPMI_PRIVILEGE_LEVEL_OPERATOR;
      else
        c->config.privilege_level = IPMI_PRIVILEGE_LEVEL_ADMIN;
    }
  else
    c->config.privilege_level = IPMI_PRIVILEGE_LEVEL_DEFAULT;

  if (ipmi_config->cipher_suite_id >= IPMI_CIPHER_SUITE_ID_MIN)
    c->config.cipher_suite_id = ipmi_config->cipher_suite_id;
  else
    c->config.cipher_suite_id = IPMI_CIPHER_SUITE_ID_DEFAULT;

  if (protocol_config->session_timeout_len > 0)
    c->config.session_timeout_len = protocol_config->session_timeout_len;
  else
    c->config.session_timeout_len = IPMICONSOLE_SESSION_TIMEOUT_LENGTH_DEFAULT;

  if (protocol_config->retransmission_timeout_len > 0)
    c->config.retransmission_timeout_len = protocol_config->retransmission_timeout_len;
  else
    c->config.retransmission_timeout_len = IPMICONSOLE_RETRANSMISSION_TIMEOUT_LENGTH_DEFAULT;

  if (protocol_config->retransmission_backoff_count > 0)
    c->config.retransmission_backoff_count = protocol_config->retransmission_backoff_count;
  else
    c->config.retransmission_backoff_count = IPMICONSOLE_RETRANSMISSION_BACKOFF_COUNT_DEFAULT;

  if (protocol_config->keepalive_timeout_len > 0)
    c->config.keepalive_timeout_len = protocol_config->keepalive_timeout_len;
  else
    c->config.keepalive_timeout_len = IPMICONSOLE_KEEPALIVE_TIMEOUT_LENGTH_DEFAULT;

  if (protocol_config->retransmission_keepalive_timeout_len > 0)
    c->config.retransmission_keepalive_timeout_len = protocol_config->retransmission_keepalive_timeout_len;
  else
    c->config.retransmission_keepalive_timeout_len = IPMICONSOLE_RETRANSMISSION_KEEPALIVE_TIMEOUT_LENGTH_DEFAULT;

  if (protocol_config->acceptable_packet_errors_count > 0)
    c->config.acceptable_packet_errors_count = protocol_config->acceptable_packet_errors_count;
  else
    c->config.acceptable_packet_errors_count = IPMICONSOLE_ACCEPTABLE_PACKET_ERRORS_COUNT_DEFAULT;

  if (protocol_config->maximum_retransmission_count > 0)
    c->config.maximum_retransmission_count = protocol_config->maximum_retransmission_count;
  else
    c->config.maximum_retransmission_count = IPMICONSOLE_MAXIMUM_RETRANSMISSION_COUNT_DEFAULT;

  /* Retransmission timeout cannot be larger than the session timeout */
  if (c->config.retransmission_timeout_len > c->config.session_timeout_len)
    {
      errno = EINVAL;
      goto cleanup;
    }

  /* Keepalive timeout cannot be larger than the session timeout */
  if (c->config.keepalive_timeout_len > c->config.session_timeout_len)
    {
      errno = EINVAL;
      goto cleanup;
    }
  
  /* Retransmission timeout cannot be larger than the keepalive timeout */
  if (c->config.retransmission_timeout_len > c->config.keepalive_timeout_len)
    {
      errno = EINVAL;
      goto cleanup;
    }

  /* Retransmission keepalive timeout cannot be larger than the keepalive timeout */
  if (c->config.retransmission_keepalive_timeout_len > c->config.keepalive_timeout_len)
    {
      errno = EINVAL;
      goto cleanup;
    }
 
  c->config.engine_flags = protocol_config->engine_flags;

  c->config.security_flags = protocol_config->security_flags;

  c->config.workaround_flags = protocol_config->workaround_flags;

  /* Data based on Configuration Parameters */

  if (ipmi_cipher_suite_id_to_algorithms(c->config.cipher_suite_id,
                                         &(c->config.authentication_algorithm),
                                         &(c->config.integrity_algorithm),
                                         &(c->config.confidentiality_algorithm)) < 0)
    {
      IPMICONSOLE_DEBUG(("ipmi_cipher_suite_id_to_algorithms: %s", strerror(errno)));
      goto cleanup;
    }

  if (ipmiconsole_ctx_debug_setup(c, protocol_config->debug_flags) < 0)
    goto cleanup;

  if (_ipmiconsole_ctx_signal_init(c) < 0)
    goto cleanup;

  if (_ipmiconsole_ctx_blocking_init(c) < 0)
    goto cleanup;

  /* only initializes value, no need to destroy/cleanup anything in here */
  _ipmiconsole_ctx_fds_init(c);

  c->session_submitted = 0;

  c->errnum = IPMICONSOLE_ERR_SUCCESS;
  return c;

 cleanup:

  ipmiconsole_ctx_debug_cleanup(c);

  _ipmiconsole_ctx_signal_cleanup(c);

  _ipmiconsole_ctx_blocking_cleanup(c);

  /* Note: use protocol_config->security_flags not c->security_flags */ 
  if (protocol_config->security_flags & IPMICONSOLE_SECURITY_LOCK_MEMORY)
    secure_free(c, sizeof(struct ipmiconsole_ctx));
  else
    free(c);
  return NULL;
}

int 
ipmiconsole_ctx_errnum(ipmiconsole_ctx_t c)
{
  if (!c)
    return IPMICONSOLE_ERR_CTX_NULL;
  else if (c->magic != IPMICONSOLE_CTX_MAGIC
           || c->api_magic != IPMICONSOLE_CTX_API_MAGIC)
    return IPMICONSOLE_ERR_CTX_INVALID;
  else
    return c->errnum;
}

char *
ipmiconsole_ctx_strerror(int errnum)
{
  if (errnum >= IPMICONSOLE_ERR_SUCCESS && errnum <= IPMICONSOLE_ERR_ERRNUMRANGE)
    return ipmiconsole_errmsgs[errnum];
  else
    return ipmiconsole_errmsgs[IPMICONSOLE_ERR_ERRNUMRANGE];
}

ipmiconsole_ctx_status_t
ipmiconsole_ctx_status(ipmiconsole_ctx_t c)
{
  int status;
  int perr;

  if (!c 
      || c->magic != IPMICONSOLE_CTX_MAGIC
      || c->api_magic != IPMICONSOLE_CTX_API_MAGIC)
    return IPMICONSOLE_CTX_STATUS_ERROR;

  /* Do not check if the context is submitted, b/c it may not be.
   *
   * Also, do not set errnum == success for this function, it could be
   * returning IPMICONSOLE_CTX_STATUS_ERROR.
   */

  if ((perr = pthread_mutex_lock(&(c->signal.status_mutex))) != 0)
    {
      IPMICONSOLE_DEBUG(("pthread_mutex_lock: %s", strerror(perr)));
      c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
      return IPMICONSOLE_CTX_STATUS_ERROR;
    }
  
  status = c->signal.status;
  
  if ((perr = pthread_mutex_unlock(&(c->signal.status_mutex))) != 0)
    {
      IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(perr)));
      c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
      return IPMICONSOLE_CTX_STATUS_ERROR;
    }

  return status;
}

int 
ipmiconsole_ctx_fd(ipmiconsole_ctx_t c)
{
  if (!c 
      || c->magic != IPMICONSOLE_CTX_MAGIC
      || c->api_magic != IPMICONSOLE_CTX_API_MAGIC)
    return -1;
  
  if (!c->session_submitted)
    {
      c->errnum = IPMICONSOLE_ERR_CTX_NOT_SUBMITTED;
      return -1;
    }

  c->errnum = IPMICONSOLE_ERR_SUCCESS;
  return c->fds.user_fd;
}

int 
ipmiconsole_ctx_generate_break(ipmiconsole_ctx_t c)
{
  uint8_t val;

  if (!c 
      || c->magic != IPMICONSOLE_CTX_MAGIC
      || c->api_magic != IPMICONSOLE_CTX_API_MAGIC)
    return -1;

  if (!c->session_submitted)
    {
      c->errnum = IPMICONSOLE_ERR_CTX_NOT_SUBMITTED;
      return -1;
    }

  val = IPMICONSOLE_PIPE_GENERATE_BREAK_CODE;
  if (write(c->fds.asynccomm[1], &val, 1) < 0)
    {
      c->errnum = IPMICONSOLE_ERR_SYSTEM_ERROR;
      return -1;
    }

  c->errnum = IPMICONSOLE_ERR_SUCCESS;
  return 0;
}

void
ipmiconsole_ctx_destroy(ipmiconsole_ctx_t c)
{
  if (!c 
      || c->magic != IPMICONSOLE_CTX_MAGIC
      || c->api_magic != IPMICONSOLE_CTX_API_MAGIC)
    return;
  
  if (c->session_submitted)
    {
      int perr;

      _ipmiconsole_ctx_fds_cleanup(c);
      _ipmiconsole_ctx_fds_init(c);

      if ((perr = pthread_mutex_lock(&(c->signal.destroyed_mutex))) != 0)
        {
          IPMICONSOLE_DEBUG(("pthread_mutex_lock: %s", strerror(perr)));
          c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
        }

      if (!c->signal.user_has_destroyed)
        c->signal.user_has_destroyed++;

      /* must change magic in this mutex, to avoid racing
       * to destroy the context.
       */
      c->api_magic = ~IPMICONSOLE_CTX_API_MAGIC;

      if ((perr = pthread_mutex_unlock(&(c->signal.destroyed_mutex))) != 0)
        {
          IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(perr)));
          c->errnum = IPMICONSOLE_ERR_INTERNAL_ERROR;
        }

      return;
    }

  /* else session never submitted, so we have to cleanup */
  c->api_magic = ~IPMICONSOLE_CTX_API_MAGIC;
  _ipmiconsole_ctx_cleanup(c);
}
