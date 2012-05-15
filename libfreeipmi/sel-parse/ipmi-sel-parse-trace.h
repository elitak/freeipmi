/*
 * Copyright (C) 2003-2012 FreeIPMI Core Team
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef IPMI_SEL_TRACE_H
#define IPMI_SEL_TRACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#ifdef STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>

#include "libcommon/ipmi-trace.h"

#define SEL_SET_ERRNUM(__ctx, __errnum)                                     \
  do {                                                                      \
    (__ctx)->errnum = (__errnum);                                           \
    TRACE_MSG_OUT (ipmi_sel_parse_ctx_errormsg ((__ctx)), (__errnum));      \
  } while (0)

#define SEL_ERRNO_TO_SEL_ERRNUM(__ctx, __errno)                             \
  do {                                                                      \
    sel_set_sel_errnum_by_errno ((__ctx), (__errno));                       \
    TRACE_ERRNO_OUT ((__errno));                                            \
  } while (0)

#define SEL_FIID_OBJECT_ERROR_TO_SEL_ERRNUM(__ctx, __obj)                   \
  do {                                                                      \
    sel_set_sel_errnum_by_fiid_object ((__ctx), (__obj));                   \
    TRACE_MSG_OUT (fiid_obj_errormsg ((__obj)), fiid_obj_errnum ((__obj))); \
  } while (0)

#endif /* IPMI_SEL_TRACE_H */
