/*
   Copyright (C) 2003-2008 FreeIPMI Core Team

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
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
*/

#ifndef _TOOL_CONFIG_FILE_COMMON_H
#define _TOOL_CONFIG_FILE_COMMON_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "tool-cmdline-common.h"
#include "conffile.h"

#define CONFIG_FILE_NONE        0x00
#define CONFIG_FILE_INBAND      0x01
#define CONFIG_FILE_OUTOFBAND   0x02
#define CONFIG_FILE_SDR         0x04
#define CONFIG_FILE_HOSTRANGE   0x08

#define CONFIG_FILE_TOOL_NONE        0x00
#define CONFIG_FILE_TOOL_IPMICONSOLE 0x01
#define CONFIG_FILE_TOOL_IPMIPOWER   0x02

struct config_file_data_ipmiconsole
{
  char escape_char;
  int escape_char_count;
  int dont_steal;
  int dont_steal_count;
  int lock_memory;
  int lock_memory_count;
};

struct config_file_data_ipmipower
{
  int on_if_off;
  int on_if_off_count;
  int wait_until_on;
  int wait_until_on_count;
  int wait_until_off;
  int wait_until_off_count;

  unsigned int retransmission_wait_timeout;
  int retransmission_wait_timeout_count;
  unsigned int retransmission_backoff_count;
  int retransmission_backoff_count_count;
  unsigned int ping_interval;
  int ping_interval_count;
  unsigned int ping_timeout;
  int ping_timeout_count;
  unsigned int ping_packet_count;
  int ping_packet_count_count;
  unsigned int ping_percent;
  int ping_percent_count;
  unsigned int ping_consec_count;
  int ping_consec_count_count;
};

int config_file_parse(const char *filename,
                      int no_error_if_not_found,
                      struct common_cmd_args *cmd_args, 
                      struct sdr_cmd_args *sdr_args, 
                      struct hostrange_cmd_args *hostrange_args, 
                      unsigned int support,
                      unsigned int tool_support,
                      void *data);

#endif
