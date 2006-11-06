/*****************************************************************************\
 *  $Id: ipmiconsole_debug.c,v 1.1 2006-11-06 00:13:12 chu11 Exp $
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
#include <stdint.h>
#if STDC_HEADERS
#include <string.h>
#include <stdarg.h>
#endif /* STDC_HEADERS */
#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#include <syslog.h>
#include <assert.h>
#include <errno.h>

#include "ipmiconsole.h"
#include "ipmiconsole_defs.h"

#include "ipmiconsole_debug.h"

#include "fd.h"

#ifndef NDEBUG
#define IPMICONSOLE_DEBUG_DIRECTORY    "/tmp"
#else  /* !NDEBUG */
#define IPMICONSOLE_DEBUG_DIRECTORY    "/var/log/ipmiconsole"
#endif /* !NDEBUG */
#define IPMICONSOLE_DEBUG_FILENAME     "ipmiconsole_debug"

static uint32_t console_debug_flags = 0;
static int console_debug_fd = -1;
static pthread_mutex_t console_stdout_debug_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t console_stderr_debug_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t console_file_debug_mutex = PTHREAD_MUTEX_INITIALIZER;

int
ipmiconsole_debug_setup(uint32_t debug_flags)
{
  assert(!(debug_flags & ~IPMICONSOLE_DEBUG_MASK));

  console_debug_flags = debug_flags;
  if (console_debug_flags & IPMICONSOLE_DEBUG_FILE)
    {
      char filename[MAXPATHLEN];
      snprintf(filename, 
	       MAXPATHLEN,
	       "%s/%s", 
	       IPMICONSOLE_DEBUG_DIRECTORY,
	       IPMICONSOLE_DEBUG_FILENAME);
  
      if ((console_debug_fd = open(filename, O_CREAT | O_APPEND | O_WRONLY, 0600)) < 0)
	{
	  console_debug_flags &= ~IPMICONSOLE_DEBUG_FILE;
	  IPMICONSOLE_DEBUG(("open: %s", strerror(errno)));
	  console_debug_flags = 0;
          return -1;
	}
    }

  return 0;
}

void
ipmiconsole_debug_cleanup(void)
{
  if (console_debug_flags & IPMICONSOLE_DEBUG_FILE && console_debug_fd)
    {
      close(console_debug_fd);
      console_debug_fd = -1;
    }
  console_debug_flags = 0;
}

int
ipmiconsole_ctx_debug_setup(ipmiconsole_ctx_t c, uint32_t debug_flags)
{
  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(!(debug_flags & ~IPMICONSOLE_DEBUG_MASK));

  c->debug_flags = debug_flags;

  if (c->debug_flags & IPMICONSOLE_DEBUG_FILE)
    {
      char filename[MAXPATHLEN];
      snprintf(filename, 
	       MAXPATHLEN,
	       "%s/%s.%s", 
	       IPMICONSOLE_DEBUG_DIRECTORY,
	       IPMICONSOLE_DEBUG_FILENAME,
	       c->hostname);
  
      if ((c->debug_fd = open(filename, O_CREAT | O_APPEND | O_WRONLY, 0600)) < 0)
	{
	  c->debug_flags &= ~IPMICONSOLE_DEBUG_FILE;
	  IPMICONSOLE_CTX_DEBUG(c, ("open: %s", strerror(errno)));
	  c->errnum = IPMICONSOLE_ERR_SYSTEM_ERROR;
	  c->debug_flags = 0;
	  return -1;
	}
    }
  
  return 0;
}

void
ipmiconsole_ctx_debug_cleanup(ipmiconsole_ctx_t c)
{
  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  
  if (c->debug_flags & IPMICONSOLE_DEBUG_FILE && c->debug_fd)
    {
      close(c->debug_fd);
      c->debug_fd = -1;
    }
  c->debug_flags = 0;
}

static void
_debug(const char *fmt, va_list ap)
{
  char errbuf[IPMICONSOLE_DEBUG_ERROR_BUFLEN];
  int len, rv;

  assert(fmt);

  len = vsnprintf(errbuf, IPMICONSOLE_DEBUG_ERROR_BUFLEN, fmt, ap);
  if (console_debug_flags & IPMICONSOLE_DEBUG_STDOUT)
    {
      if ((rv = pthread_mutex_lock(&console_stdout_debug_mutex)))
        {
          console_debug_flags &= ~IPMICONSOLE_DEBUG_STDOUT;
          IPMICONSOLE_DEBUG(("pthread_mutex_lock: %s", strerror(rv)));
          goto try_stderr;
        }

      fprintf(stdout, "%s\r\n", errbuf);
      fflush(stdout);

      if ((rv = pthread_mutex_unlock(&console_stdout_debug_mutex)))
        {
          console_debug_flags &= ~IPMICONSOLE_DEBUG_STDOUT;
          IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(rv)));      
          goto try_stderr;
        }
    }
 try_stderr:
  if (console_debug_flags & IPMICONSOLE_DEBUG_STDERR)
    {
      if ((rv = pthread_mutex_lock(&console_stderr_debug_mutex)))
        {
          console_debug_flags &= ~IPMICONSOLE_DEBUG_STDERR;
          IPMICONSOLE_DEBUG(("pthread_mutex_lock: %s", strerror(rv)));
          goto try_syslog;
        }

      fprintf(stderr, "%s\r\n", errbuf);
      fflush(stderr);

      if ((rv = pthread_mutex_unlock(&console_stderr_debug_mutex)))
        {
          console_debug_flags &= ~IPMICONSOLE_DEBUG_STDERR;
          IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(rv)));      
          goto try_syslog;
        }
    }
 try_syslog:
  if (console_debug_flags & IPMICONSOLE_DEBUG_SYSLOG)
    syslog(LOG_DEBUG, "%s", errbuf);
  if (console_debug_flags & IPMICONSOLE_DEBUG_FILE)
    {
      char tbuf[IPMICONSOLE_DEBUG_ERROR_BUFLEN+2];
      int tlen;

      tlen = snprintf(tbuf, IPMICONSOLE_DEBUG_ERROR_BUFLEN+2, "%s\n", errbuf);

      if ((rv = pthread_mutex_lock(&console_file_debug_mutex)))
        {
          console_debug_flags &= ~IPMICONSOLE_DEBUG_FILE;
          IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(rv)));      
          goto out;
        }

      if ((fd_write_n(console_debug_fd, tbuf, tlen)) < 0)
	{
	  console_debug_flags &= ~IPMICONSOLE_DEBUG_FILE;
	  IPMICONSOLE_DEBUG(("fd_write_n: %s", strerror(errno)));
          /* fall-through to try and unlock */
	}

      if ((rv = pthread_mutex_unlock(&console_file_debug_mutex)))
        {
          console_debug_flags &= ~IPMICONSOLE_DEBUG_FILE;
          IPMICONSOLE_DEBUG(("pthread_mutex_unlock: %s", strerror(rv)));      
          goto out;
        }
    }
 out:
  /* Shutup gcc */
  ;
}

void
ipmiconsole_debug(const char *fmt, ...)
{
  va_list ap;

  assert(fmt);

  va_start(ap, fmt);
  _debug(fmt, ap);
  va_end(ap);
}

static void
_ctx_debug(ipmiconsole_ctx_t c, const char *fmt, va_list ap)
{
  char errbuf[IPMICONSOLE_DEBUG_ERROR_BUFLEN];
  int len, rv;

  assert(fmt);

  len = vsnprintf(errbuf, IPMICONSOLE_DEBUG_ERROR_BUFLEN, fmt, ap);
  if (c->debug_flags & IPMICONSOLE_DEBUG_STDOUT)
    {
      if ((rv = pthread_mutex_lock(&console_stdout_debug_mutex)))
        {
          c->debug_flags &= ~IPMICONSOLE_DEBUG_STDOUT;
          IPMICONSOLE_CTX_DEBUG(c, ("pthread_mutex_lock: %s", strerror(rv)));
          goto try_stderr;
        }

      fprintf(stdout, "%s\r\n", errbuf);
      fflush(stdout);

      if ((rv = pthread_mutex_unlock(&console_stdout_debug_mutex)))
        {
          c->debug_flags &= ~IPMICONSOLE_DEBUG_STDOUT;
          IPMICONSOLE_CTX_DEBUG(c, ("pthread_mutex_unlock: %s", strerror(rv)));      
          goto try_stderr;
        }
    }
 try_stderr:
  if (c->debug_flags & IPMICONSOLE_DEBUG_STDERR)
    {
      if ((rv = pthread_mutex_lock(&console_stderr_debug_mutex)))
        {
          c->debug_flags &= ~IPMICONSOLE_DEBUG_STDERR;
          IPMICONSOLE_CTX_DEBUG(c, ("pthread_mutex_lock: %s", strerror(rv)));
          goto try_syslog;
        }

      fprintf(stderr, "%s\r\n", errbuf);
      fflush(stderr);

      if ((rv = pthread_mutex_unlock(&console_stderr_debug_mutex)))
        {
          c->debug_flags &= ~IPMICONSOLE_DEBUG_STDERR;
          IPMICONSOLE_CTX_DEBUG(c, ("pthread_mutex_unlock: %s", strerror(rv)));      
          goto try_syslog;
        }
    }
 try_syslog:
  if (c->debug_flags & IPMICONSOLE_DEBUG_SYSLOG)
    syslog(LOG_DEBUG, "%s", errbuf);
  if (c->debug_flags & IPMICONSOLE_DEBUG_FILE)
    {
      char tbuf[IPMICONSOLE_DEBUG_ERROR_BUFLEN+2];
      int tlen;

      tlen = snprintf(tbuf, IPMICONSOLE_DEBUG_ERROR_BUFLEN+2, "%s\n", errbuf);

      /* Note: This is a per-ctx file descriptor, so thread syncing
       * isn't required
       */
      if ((fd_write_n(c->debug_fd, tbuf, tlen)) < 0)
	{
	  c->debug_flags &= ~IPMICONSOLE_DEBUG_FILE;
	  IPMICONSOLE_CTX_DEBUG(c, ("fd_write_n: %s", strerror(errno)));
	}
    }
}

void
ipmiconsole_ctx_debug(ipmiconsole_ctx_t c, const char *fmt, ...)
{
  va_list ap;

  assert(c);
  assert(c->magic == IPMICONSOLE_CTX_MAGIC);
  assert(fmt);

  va_start(ap, fmt);
  _ctx_debug(c, fmt, ap);
  va_end(ap);
}

char * 
__debug_msg_create(const char *fmt, ...)
{
  char *buffer;
  va_list ap;

  assert(fmt);

  if (!(buffer = malloc(IPMICONSOLE_DEBUG_ERROR_BUFLEN)))
    return NULL;

  va_start(ap, fmt);
  vsnprintf(buffer, IPMICONSOLE_DEBUG_ERROR_BUFLEN, fmt, ap);
  va_end(ap);

  return buffer;
}
