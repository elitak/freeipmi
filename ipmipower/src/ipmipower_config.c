/*****************************************************************************\
 *  $Id: ipmipower_config.c,v 1.98 2008-05-15 20:22:55 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2008 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2003-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>
 *  UCRL-CODE-155698
 *  
 *  This file is part of Ipmipower, a remote power control utility.
 *  For details, see http://www.llnl.gov/linux/.
 *  
 *  Ipmipower is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by the 
 *  Free Software Foundation; either version 2 of the License, or (at your 
 *  option) any later version.
 *  
 *  Ipmipower is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 *  for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with Ipmipower.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <assert.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <stdint.h>
#if HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include <errno.h>

#include <argp.h>

#include "ipmipower_config.h"
#include "ipmipower_output.h"
#include "ipmipower_util.h"
#include "ipmipower_workarounds.h"
#include "ipmipower_wrappers.h"

#include "secure.h"
#include "freeipmi-portability.h"
#include "pstdout.h"
#include "tool-common.h"
#include "tool-cmdline-common.h"
      
extern struct ipmipower_config *conf;
extern struct ipmipower_connection *ics;

const char *argp_program_version = "ipmipower " VERSION "\n";

const char *argp_program_bug_address = "<freeipmi-devel@gnu.org>";

#define IPMI_VERSION_KEY                 'R'

#define RETRY_TIMEOUT_KEY                160
#define RETRANSMISSION_TIMEOUT_KEY       'y'
#define TIMEOUT_KEY                      161
#define SESSION_TIMEOUT_KEY              't'
#define CONFIG_KEY                       164
#define DEBUG_KEY                        165
#define IPMIDUMP_KEY                     166
#define RMCPDUMP_KEY                     167
#define LOG_KEY                          168
#define LOGFILE_KEY                      169

#define ON_KEY                           'n'
#define OFF_KEY                          'f'
#define CYCLE_KEY                        'c'
#define RESET_KEY                        'r'
#define STAT_KEY                         's'
#define PULSE_KEY                        'j'
#define SOFT_KEY                         'm'
#define ON_IF_OFF_KEY                    'g'
#define WAIT_UNTIL_OFF_KEY               'A'
#define WAIT_UNTIL_ON_KEY                'G'

#define RETRY_WAIT_TIMEOUT_KEY           162
#define RETRANSMISSION_WAIT_TIMEOUT_KEY  'q'
#define RETRY_BACKOFF_COUNT_KEY          163
#define RETRANSMISSION_BACKOFF_COUNT_KEY 'b'
#define PING_INTERVAL_KEY                'i'
#define PING_TIMEOUT_KEY                 'z'
#define PING_PACKET_COUNT_KEY            'v'
#define PING_PERCENT_KEY                 'w'
#define PING_CONSEC_COUNT_KEY            'x'

static struct argp_option cmdline_options[] =
  {
    ARGP_COMMON_OPTIONS_DRIVER,
    /* maintain "ipmi-version" for backwards compatability */
    {"ipmi-version", IPMI_VERSION_KEY, "IPMIVERSION", OPTION_HIDDEN,
     "Specify the IPMI protocol version to use.", 11},
    ARGP_COMMON_OPTIONS_OUTOFBAND_HOSTRANGED_NO_TIMEOUT,
    /* maintain "retry-timeout" for backwards compatability */
    {"retry-timeout", RETRY_TIMEOUT_KEY, "MILLISECONDS", OPTION_HIDDEN,
     "Specify the packet retransmission timeout in milliseconds.", 11},
    /* don't use common cmdline parsing headers for retransmission-timeout, we need to support
     * backwards compatible short options
     */
    {"retransmission-timeout", RETRANSMISSION_TIMEOUT_KEY, "MILLISECONDS", 0,
     "Specify the packet retransmission timeout in milliseconds.", 11},
    /* maintain "timeout" for backwards compatability */
    {"timeout", TIMEOUT_KEY, "MILLISECONDS", OPTION_HIDDEN,
     "Specify the session timeout in milliseconds.", 12},
    {"session-timeout", SESSION_TIMEOUT_KEY, "MILLISECONDS", 0,
     "Specify the session timeout in milliseconds.", 12},
    ARGP_COMMON_OPTIONS_AUTHENTICATION_TYPE,
    ARGP_COMMON_OPTIONS_CIPHER_SUITE_ID,
    /* maintain "privilege" for backwards compatability */
    {"privilege",  ARGP_PRIVILEGE_KEY, "PRIVILEGE-LEVEL", OPTION_HIDDEN,                            
     "Specify the privilege level to be used.", 15},
    {"privilege-level",  ARGP_PRIVILEGE_LEVEL_KEY, "PRIVILEGE-LEVEL", 0,
     "Specify the privilege level to be used.", 15},
    ARGP_COMMON_OPTIONS_WORKAROUND_FLAGS,
    ARGP_COMMON_HOSTRANGED_OPTIONS,
    {"debug", DEBUG_KEY, 0, 0,
     "Turn on debugging.", 25},
    {"config", CONFIG_KEY, "FILE", 0,
     "Specify an alternate configuration file.", 26},
#ifndef NDEBUG
    {"rmcpdump", RMCPDUMP_KEY, 0, 0,
     "Turn on RMCP packet dump output.", 27},
    {"log", LOG_KEY, 0, 0,
     "Turn on logging.", 28},
    {"logfile", LOGFILE_KEY, "FILE", 0,
     "Specify an alternate logfile.", 29},
#endif
    {"on", ON_KEY, 0, 0,
     "Power on the target hosts.", 30},
    {"off", OFF_KEY, 0, 0,
     "Power off the target hosts.", 31},
    {"cycle", CYCLE_KEY, 0, 0,
     "Power cycle the target hosts.", 32},
    {"reset", RESET_KEY, 0, 0,
     "Reset the target hosts.", 33},
    {"stat", STAT_KEY, 0, 0,
     "Get power status of the target hosts.", 34},
    {"pulse", PULSE_KEY, 0, 0,
     "Send power diagnostic interrupt to target hosts.", 35},
    {"soft", SOFT_KEY, 0, 0,
     "Initiate a soft-shutdown of the OS via ACPI.", 36},
    {"on-if-off", ON_IF_OFF_KEY, 0, 0,
     "Issue a power on command instead of a power cycle or hard reset "
     "command if the remote machine's power is currently off.", 38},
    {"wait-until-off", WAIT_UNTIL_OFF_KEY, 0, 0,
     "Regularly query the remote BMC and return only after the machine has powered off.", 39},
    {"wait-until-on", WAIT_UNTIL_ON_KEY, 0, 0,
     "Regularly query the remote BMC and return only after the machine has powered on.", 40},
    /* retry-wait-timeout maintained for backwards comptability */
    {"retry-wait-timeout", RETRY_WAIT_TIMEOUT_KEY, "MILLISECONDS", OPTION_HIDDEN,
     "Specify the retransmission timeout length in milliseconds.", 41},
    {"retransmission-wait-timeout", RETRANSMISSION_WAIT_TIMEOUT_KEY, "MILLISECONDS", 0,
     "Specify the retransmission timeout length in milliseconds.", 41},
    /* retry-backoff-count maintained for backwards comptability */
    {"retry-backoff-count", RETRY_BACKOFF_COUNT_KEY, "COUNT", OPTION_HIDDEN,
     "Specify the retransmission backoff count for retransmissions.", 42},
    {"retransmission-backoff-count", RETRANSMISSION_BACKOFF_COUNT_KEY, "COUNT", 0,
     "Specify the retransmission backoff count for retransmissions.", 42},
    {"ping-interval", PING_INTERVAL_KEY, "MILLISECONDS", 0,
     "Specify the ping interval length in milliseconds.", 43},
    {"ping-timeout", PING_TIMEOUT_KEY, "MILLISECONDS", 0,
     "Specify the ping timeout length in milliseconds.", 44},
    {"ping-packet-count", PING_PACKET_COUNT_KEY, "COUNT", 0,
     "Specify the ping packet count size.", 45},
    {"ping-percent", PING_PERCENT_KEY, "PERCENT", 0,
     "Specify the ping percent value.", 46},
    {"ping-consec-count", PING_CONSEC_COUNT_KEY, "COUNT", 0,
     "Specify the ping consecutive count.", 47},
    { 0 }
  };

static error_t cmdline_parse (int key, char *arg, struct argp_state *state);

static char cmdline_args_doc[] = "";

static char cmdline_doc[] = "ipmipower - IPMI power control utility";

static struct argp cmdline_argp = {cmdline_options,
                                   cmdline_parse,
                                   cmdline_args_doc,
                                   cmdline_doc};


void 
ipmipower_config_default_logfile(char *buf, int buflen)
{
  char buffer[MAXPATHLEN+1];
  pid_t pid;

  assert(buf != NULL && buflen > 0);

  pid = getpid();
  snprintf(buffer, MAXPATHLEN, IPMIPOWER_DEFAULT_LOGFILE, pid);
  if (strlen(buffer) > buflen - 1)
    ierr_exit("ipmipower_config_default_logfile: internal buffer too small");
  strcpy(buf, buffer);
}

void 
ipmipower_config_setup(void) 
{
  assert(conf == NULL);         /* Already initialized */

  if (!(conf = (struct ipmipower_config *)malloc(sizeof(struct ipmipower_config))))
    ierr_exit("malloc: %s", strerror(errno));
  
  conf->driver_type = IPMI_DEVICE_LAN;
  conf->hosts = NULL;
  conf->hosts_count = 0;
  memset(conf->username, '\0', IPMI_MAX_USER_NAME_LENGTH+1);
  memset(conf->password, '\0', IPMI_2_0_MAX_PASSWORD_LENGTH+1);
  memset(conf->k_g, '\0', IPMI_MAX_K_G_LENGTH+1);
  conf->k_g_len = 0;
  conf->session_timeout_len = 20000;     /* 20 seconds */
  conf->retransmission_timeout_len = 400; /* .4 seconds  */
  conf->authentication_type = IPMI_AUTHENTICATION_TYPE_MD5;
  conf->cipher_suite_id = 3;
  conf->privilege_level = IPMI_PRIVILEGE_LEVEL_OPERATOR;
  conf->workaround_flags = 0;
  conf->debug = 0;
  memset(conf->configfile, '\0', MAXPATHLEN+1);
#ifndef NDEBUG
  conf->rmcpdump = 0;
  conf->log = 0;
  memset(conf->logfile, '\0', MAXPATHLEN+1);
  ipmipower_config_default_logfile(conf->logfile, MAXPATHLEN);
  conf->logfile_fd = -1;
#endif /* NDEBUG */
  conf->buffer_output = 0;
  conf->consolidate_output = 0;
  conf->fanout = 0;
  conf->eliminate = 0;
  conf->always_prefix = 0;

  conf->powercmd = POWER_CMD_NONE;
  conf->on_if_off = 0;
  conf->wait_until_on = 0;
  conf->wait_until_off = 0;
  conf->retransmission_wait_timeout_len = 500; /* .5 seconds  */
  conf->retransmission_backoff_count = 8;
  conf->ping_interval_len = 5000; /* 5 seconds */
  conf->ping_timeout_len = 30000; /* 30 seconds */
  conf->ping_packet_count = 10;
  conf->ping_percent = 50;
  conf->ping_consec_count = 5;

  /* Options not found yet, all false */
  conf->driver_type_set_on_cmdline = 0;
  conf->hosts_set_on_cmdline = 0;
  conf->username_set_on_cmdline = 0;
  conf->password_set_on_cmdline = 0;
  conf->session_timeout_len_set_on_cmdline = 0;
  conf->retransmission_timeout_len_set_on_cmdline = 0;
  conf->authentication_type_set_on_cmdline = 0;
  conf->cipher_suite_id_set_on_cmdline = 0;
  conf->privilege_level_set_on_cmdline = 0;
  conf->workaround_flags_set_on_cmdline = 0;
  conf->buffer_output_set_on_cmdline = 0;
  conf->consolidate_output_set_on_cmdline = 0;
  conf->fanout_set_on_cmdline = 0;
  conf->eliminate_set_on_cmdline = 0;
  conf->always_prefix_set_on_cmdline = 0;
  conf->on_if_off_set_on_cmdline = 0;
  conf->wait_until_on_set_on_cmdline = 0;
  conf->wait_until_off_set_on_cmdline = 0;
  conf->retransmission_wait_timeout_len_set_on_cmdline = 0;
  conf->retransmission_backoff_count_set_on_cmdline = 0;
  conf->ping_interval_len_set_on_cmdline = 0;
  conf->ping_timeout_len_set_on_cmdline = 0;
  conf->ping_packet_count_set_on_cmdline = 0;
  conf->ping_percent_set_on_cmdline = 0;
  conf->ping_consec_count_set_on_cmdline = 0;
}

static void 
_config_common_checks(char *str) 
{
  assert (str != NULL);

  if (conf->hosts != NULL 
      && (conf->hosts_count < IPMIPOWER_MINNODES 
          || conf->hosts_count > IPMIPOWER_MAXNODES))
    ierr_exit("%s: invalid number of hostnames", str);
    
  if (conf->session_timeout_len < IPMIPOWER_SESSION_TIMEOUT_MIN 
      || conf->session_timeout_len > IPMIPOWER_SESSION_TIMEOUT_MAX)
    ierr_exit("%s: timeout out of range", str);
  
  if (conf->retransmission_timeout_len != 0 
      && (conf->retransmission_timeout_len < IPMIPOWER_RETRANSMISSION_TIMEOUT_MIN 
          || conf->retransmission_timeout_len > IPMIPOWER_RETRANSMISSION_TIMEOUT_MAX))
    ierr_exit("%s: retransmission timeout out of range", str);

  if (!IPMI_CIPHER_SUITE_ID_SUPPORTED(conf->cipher_suite_id))
    ierr_exit("%s: invalid cipher suite id", str);

  if (conf->retransmission_wait_timeout_len != 0 
      && (conf->retransmission_wait_timeout_len < IPMIPOWER_RETRANSMISSION_WAIT_TIMEOUT_MIN 
          || conf->retransmission_wait_timeout_len > IPMIPOWER_RETRANSMISSION_WAIT_TIMEOUT_MAX))
    ierr_exit("%s: retransmission wait timeout out of range", str);
  
  if (conf->retransmission_backoff_count != 0 
      && (conf->retransmission_backoff_count < IPMIPOWER_RETRANSMISSION_BACKOFF_COUNT_MIN 
          || conf->retransmission_backoff_count > IPMIPOWER_RETRANSMISSION_BACKOFF_COUNT_MAX))
    ierr_exit("%s: retransmission backoff count out of range", str);

  if (conf->ping_interval_len != 0 
      && (conf->ping_interval_len < IPMIPOWER_PING_INTERVAL_MIN 
          || conf->ping_interval_len > IPMIPOWER_PING_INTERVAL_MAX))
    ierr_exit("%s: ping interval out of range", str);
  
  if (conf->ping_timeout_len != 0 
      && (conf->ping_timeout_len < IPMIPOWER_PING_TIMEOUT_MIN 
          || conf->ping_timeout_len > IPMIPOWER_PING_TIMEOUT_MAX))
    ierr_exit("%s: ping timeout out of range", str);

  if (conf->ping_packet_count != 0
      && (conf->ping_packet_count < IPMIPOWER_PING_PACKET_COUNT_MIN
          || conf->ping_packet_count > IPMIPOWER_PING_PACKET_COUNT_MAX))
    ierr_exit("%s: ping packet out of range", str);

  if (conf->ping_percent != 0
      && (conf->ping_percent < IPMIPOWER_PING_PERCENT_MIN
          || conf->ping_percent > IPMIPOWER_PING_PERCENT_MAX))
    ierr_exit("%s: ping percent out of range", str);
  
  if (conf->ping_consec_count != 0
      && (conf->ping_consec_count < IPMIPOWER_PING_CONSEC_COUNT_MIN
          || conf->ping_consec_count > IPMIPOWER_PING_CONSEC_COUNT_MAX))
    ierr_exit("%s: ping consec out of range", str);
}

static error_t
cmdline_parse (int key,
               char *arg,
               struct argp_state *state)
{
  char *ptr;
  char *pw;
  char *kg;
  int rv;
  uint32_t flags;
  int n;
  int tmp;

  switch (key) 
    {
    /* IPMI_VERSION_KEY for backwards compatability */
    case IPMI_VERSION_KEY:	/* --ipmi-version */
      if (!strcasecmp(arg, "1.5"))
        tmp = IPMI_DEVICE_LAN;
      else if (!strcasecmp(arg, "2.0"))
        tmp = IPMI_DEVICE_LAN_2_0;
      else
        ierr_exit("Command Line Error: invalid driver type specified");
      conf->driver_type = tmp;
      conf->driver_type_set_on_cmdline++;
      break;
    case ARGP_DRIVER_TYPE_KEY:      /* --driver-type */
      if ((tmp = parse_outofband_driver_type(arg)) < 0)
        ierr_exit("Command Line Error: invalid driver type specified");
      conf->driver_type = tmp;
      conf->driver_type_set_on_cmdline++;
      break;
    case ARGP_HOSTNAME_KEY:       /* --hostname */
      if ((conf->hosts = hostlist_create(arg)) == NULL)
        ierr_exit("Command Line Error: Hostname(s) incorrectly formatted");
      hostlist_uniq(conf->hosts);
      conf->hosts_count = hostlist_count(conf->hosts);
      conf->hosts_set_on_cmdline++;
      break;
    case ARGP_USERNAME_KEY:       /* --username */
      if (strlen(arg) > IPMI_MAX_USER_NAME_LENGTH)
        ierr_exit("Command Line Error: username too long");
      strcpy(conf->username, arg);
      conf->username_set_on_cmdline++;
      n = strlen(arg);
      secure_memset(arg, '\0', n);
      break;
    case ARGP_PASSWORD_KEY:       /* --password */
      if (strlen(arg) > IPMI_2_0_MAX_PASSWORD_LENGTH)
        ierr_exit("Command Line Error: password too long");
      strcpy(conf->password, arg);
      conf->password_set_on_cmdline++;
      n = strlen(arg);
      secure_memset(arg, '\0', n);
      break;
    case ARGP_PASSWORD_PROMPT_KEY:       /* --password-prompt */
      if (!(pw = getpass("Password: ")))
        ierr_exit("getpass: %s", strerror(errno));
      if (strlen(pw) > IPMI_2_0_MAX_PASSWORD_LENGTH)
        ierr_exit("password too long");
      strcpy(conf->password, pw);
      conf->password_set_on_cmdline++;
      break;
    case ARGP_K_G_KEY:       /* --k-g */
      if ((rv = check_kg_len(arg)) < 0)
        ierr_exit("Command Line Error: k_g too long");
      if ((rv = parse_kg(conf->k_g, IPMI_MAX_K_G_LENGTH + 1, arg)) < 0)
        ierr_exit("Command Line Error: k_g input formatted incorrectly");
      if (rv > 0)
        {
          conf->k_g_len = rv;
          conf->k_g_set_on_cmdline++;
        }
      n = strlen(arg);
      secure_memset(arg, '\0', n);
      break;
    case ARGP_K_G_PROMPT_KEY:       /* --k-g-prompt */
      if (!(kg = getpass("K_g: ")))
        ierr_exit("getpass: %s", strerror(errno));
      if ((rv = check_kg_len(kg)) < 0)
        ierr_exit("Command Line Error: k_g too long");
      if ((rv = parse_kg(conf->k_g, IPMI_MAX_K_G_LENGTH + 1, kg)) < 0)
        ierr_exit("Command Line Error: k_g input formatted incorrectly");
      if (rv > 0)
        {
          conf->k_g_len = rv;
          conf->k_g_set_on_cmdline++;
        }
      break;
    case SESSION_TIMEOUT_KEY:       /* --session-timeout */
      conf->session_timeout_len = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: session timeout length invalid");
      conf->session_timeout_len_set_on_cmdline++;
      break;
    case RETRANSMISSION_TIMEOUT_KEY:       /* --retransmission-timeout */
      conf->retransmission_timeout_len = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: retransmission timeout length invalid");
      conf->retransmission_timeout_len_set_on_cmdline++;
      break;
    case ARGP_AUTHENTICATION_TYPE_KEY:       /* --authentication-type */
      if ((tmp = parse_authentication_type(arg)) < 0)
        ierr_exit("Command Line Error: invalid authentication type specified");
      conf->authentication_type = tmp;
      conf->authentication_type_set_on_cmdline++;
      break;
    case ARGP_CIPHER_SUITE_ID_KEY:       /* --cipher-suite-id */
      conf->cipher_suite_id = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: invalid cipher suite id");
      conf->cipher_suite_id_set_on_cmdline++;
      break;
      /* ARGP_PRIVILEGE_KEY for backwards compatability */
    case ARGP_PRIVILEGE_KEY:
    case ARGP_PRIVILEGE_LEVEL_KEY:       /* --privilege-level */
      if ((tmp = parse_privilege_level(arg)) < 0)
        ierr_exit("Command Line Error: invalid privilege level specified");
      conf->privilege_level = tmp;
      conf->privilege_level_set_on_cmdline++;
      break;
    case ARGP_WORKAROUND_FLAGS_KEY:       /* --workaround-flags */
      if (ipmipower_workarounds_parse(arg, &flags) < 0)
        ierr_exit("Command Line Error: invalid workaround specified");
      conf->workaround_flags = flags;
      conf->workaround_flags_set_on_cmdline++;
      break;
    case DEBUG_KEY:          /* --debug */
      conf->debug++;
      break;
    case CONFIG_KEY:         /* --config */
      if (strlen(arg) > MAXPATHLEN)
        ierr_exit("Command Line Error: configuration file pathname too long");
      strcpy(conf->configfile, arg);
      break;
#ifndef NDEBUG
    case RMCPDUMP_KEY:       /* --rmcpdump */
      conf->rmcpdump++;
      break;
    case LOG_KEY:            /* --log */
      conf->log++;
      break;
    case LOGFILE_KEY:        /* --logfile */
      if (strlen(arg) > MAXPATHLEN)
        ierr_exit("Command Line Error: log file pathname too long");
      memset(conf->logfile, '\0', MAXPATHLEN+1);
      strcpy(conf->logfile, arg);
      break;
#endif /* !NDEBUG */
    case ARGP_BUFFER_OUTPUT_KEY:       /* --buffer-output */
      conf->buffer_output++;
      conf->buffer_output_set_on_cmdline++;
      break;
    case ARGP_CONSOLIDATE_OUTPUT_KEY:       /* --consolidate-output */
      conf->consolidate_output++;
      conf->consolidate_output_set_on_cmdline++;
      break;
    case ARGP_FANOUT_KEY:          /* --fanout */
      conf->fanout = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: fanout invalid");
      conf->fanout_set_on_cmdline++;
      break;
    case ARGP_ELIMINATE_KEY:       /* --eliminate */
      conf->eliminate++;
      conf->eliminate_set_on_cmdline++;
      break;
    case ARGP_ALWAYS_PREFIX_KEY: /* --always-prefix */
      conf->always_prefix++;
      conf->always_prefix_set_on_cmdline++;
      break;
    case ON_KEY:       /* --on */ 
      conf->powercmd = POWER_CMD_POWER_ON;
      break;
    case OFF_KEY:       /* --off */ 
      conf->powercmd = POWER_CMD_POWER_OFF;
      break;
    case CYCLE_KEY:       /* --cycle */ 
      conf->powercmd = POWER_CMD_POWER_CYCLE;
      break;
    case RESET_KEY:       /* --reset */ 
      conf->powercmd = POWER_CMD_POWER_RESET;
      break;
    case STAT_KEY:       /* --stat */ 
      conf->powercmd = POWER_CMD_POWER_STATUS;
      break;
    case PULSE_KEY:       /* --pulse */
      conf->powercmd = POWER_CMD_PULSE_DIAG_INTR;
      break;
    case SOFT_KEY:       /* --soft */
      conf->powercmd = POWER_CMD_SOFT_SHUTDOWN_OS;
      break;
    case ON_IF_OFF_KEY:       /* --on-if-off */
      conf->on_if_off++;
      conf->on_if_off_set_on_cmdline++;
      break;
    case WAIT_UNTIL_OFF_KEY:       /* --wait-until-on */
      conf->wait_until_on++;
      conf->wait_until_on_set_on_cmdline++;
      break;
    case WAIT_UNTIL_ON_KEY:       /* --wait-until-off */
      conf->wait_until_off++;
      conf->wait_until_off_set_on_cmdline++;
      break;
      /* RETRY_WAIT_TIMEOUT for backwards compatability */
    case RETRY_WAIT_TIMEOUT_KEY:
    case RETRANSMISSION_WAIT_TIMEOUT_KEY:       /* --retransmission-wait-timeout */
      conf->retransmission_wait_timeout_len = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: retransmission wait timeout length invalid");
      conf->retransmission_wait_timeout_len_set_on_cmdline++;
      break;
      /* RETRY_BACKOFF_COUNT for backwards compatability */
    case RETRY_BACKOFF_COUNT_KEY:
    case RETRANSMISSION_BACKOFF_COUNT_KEY:       /* --retransmission-backoff-count */
      conf->retransmission_backoff_count = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: retransmission backoff count invalid");
      conf->retransmission_backoff_count_set_on_cmdline++;
      break;
    case PING_INTERVAL_KEY:       /* --ping-interval */
      conf->ping_interval_len = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: ping interval length invalid");
      conf->ping_interval_len_set_on_cmdline++;
      break;
    case PING_TIMEOUT_KEY:       /* --ping-timeout */
      conf->ping_timeout_len = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: ping timeout length invalid");
      conf->ping_timeout_len_set_on_cmdline++;
      break;
    case PING_PACKET_COUNT_KEY:       /* --ping-packet-count */
      conf->ping_packet_count = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: ping packet count invalid");
      conf->ping_packet_count_set_on_cmdline++;
      break;
    case PING_PERCENT_KEY:       /* --ping-percent */
      conf->ping_percent = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: ping percent invalid");
      conf->ping_percent_set_on_cmdline++;
      break;
    case PING_CONSEC_COUNT_KEY:       /* --ping-consec-count */
      conf->ping_consec_count = strtol(arg, &ptr, 10);
      if (ptr != (arg + strlen(arg)))
        ierr_exit("Command Line Error: ping consec count invalid");
      conf->ping_consec_count_set_on_cmdline++;
      break;
    case '?':
    default:
      return ARGP_ERR_UNKNOWN;
    } 

  return 0;
}

static void
post_cmdline_parse_verify(void)
{
  _config_common_checks("Command Line Error");
  
  if (conf->powercmd != POWER_CMD_NONE)
    conf->ping_interval_len = 0;     /* force pings to be off */
}

void 
ipmipower_config_cmdline_parse(int argc, char **argv)
{
  argp_parse(&cmdline_argp, argc, argv, ARGP_IN_ORDER, NULL, NULL);
  post_cmdline_parse_verify();
}

/*
 * Conffile library callback functions
 */
    
static int 
_cb_driver_type(conffile_t cf, struct conffile_data *data,
                char *optionname, int option_type, void *option_ptr, 
                int option_data, void *app_ptr, int app_data) 
{
  int tmp;

  if (conf->driver_type_set_on_cmdline)
    return 0;

  if ((tmp = parse_outofband_driver_type(data->string)) < 0)
    ierr_exit("Config File Error: invalid driver type specified");

  conf->driver_type = tmp;
  return 0;
}

static int 
_cb_hostname(conffile_t cf, struct conffile_data *data,
             char *optionname, int option_type, void *option_ptr, 
             int option_data, void *app_ptr, int app_data) 
{
  int i;
  
  if (conf->hosts_set_on_cmdline)
    return 0;
  
  if ((conf->hosts = hostlist_create(NULL)) == NULL)
    ierr_exit("Config File Error: Hostname(s) incorrectly formatted");
  
  for (i = 0; i < data->stringlist_len; i++) 
    {
      if (hostlist_push(conf->hosts, data->stringlist[i]) == 0)
        ierr_exit("Config File Error: Hostname(s) incorrectly formatted");
    }
  
  hostlist_uniq(conf->hosts);
  
  conf->hosts_count = hostlist_count(conf->hosts);
  
  return 0;
}

static int 
_cb_username(conffile_t cf, struct conffile_data *data,
             char *optionname, int option_type, void *option_ptr,
             int option_data, void *app_ptr, int app_data) 
{
  if (conf->username_set_on_cmdline)
    return 0;

  if (strlen(data->string) > IPMI_MAX_USER_NAME_LENGTH)
    ierr_exit("Config File Error: username too long");

  strcpy(conf->username, data->string);
  return 0;
}

static int 
_cb_password(conffile_t cf, struct conffile_data *data,
             char *optionname, int option_type, void *option_ptr,
             int option_data, void *app_ptr, int app_data) 
{
  if (conf->password_set_on_cmdline)
    return 0;

  if (strlen(data->string) > IPMI_2_0_MAX_PASSWORD_LENGTH)
    ierr_exit("Config File Error: password too long");

  strcpy(conf->password, data->string);
  return 0;
}

static int 
_cb_k_g(conffile_t cf, struct conffile_data *data,
        char *optionname, int option_type, void *option_ptr,
        int option_data, void *app_ptr, int app_data) 
{
  int rv;

  if (conf->k_g_set_on_cmdline)
    return 0;

  if ((rv = check_kg_len(data->string)) < 0)
    ierr_exit("Command Line Error: k_g too long");

  if ((rv = parse_kg(conf->k_g, IPMI_MAX_K_G_LENGTH + 1, data->string)) < 0)
    ierr_exit("Config File Error: k_g input formatted incorrectly");

  if (rv > 0)
    conf->k_g_len = rv;

  return 0;
}

static int 
_cb_authentication_type(conffile_t cf, struct conffile_data *data,
			char *optionname, int option_type, void *option_ptr, 
			int option_data, void *app_ptr, int app_data) 
{
  int tmp;

  if (conf->authentication_type_set_on_cmdline)
    return 0;

  if ((tmp = parse_authentication_type(data->string)) < 0)
    ierr_exit("Config File Error: invalid authentication type specified");

  conf->authentication_type = tmp;
  return 0;
}

static int 
_cb_privilege_level(conffile_t cf, struct conffile_data *data,
                    char *optionname, int option_type, void *option_ptr, 
                    int option_data, void *app_ptr, int app_data) 
{
  int tmp;
  
  if (conf->privilege_level_set_on_cmdline)
    return 0;
  
  if ((tmp = parse_privilege_level(data->string)) < 0)
    ierr_exit("Config File Error: invalid privilege level specified");

  conf->privilege_level = tmp;
  return 0;
}

static int 
_cb_workaround_flags(conffile_t cf, struct conffile_data *data,
                     char *optionname, int option_type, void *option_ptr,
                     int option_data, void *app_ptr, int app_data) 
{
  uint32_t flags;

  if (conf->workaround_flags_set_on_cmdline)
    return 0;

  if (ipmipower_workarounds_parse(data->string, &flags) < 0)
    ierr_exit("Config File Error: invalid workaround specified");
  conf->workaround_flags = flags;
  return 0;
}

static int 
_cb_bool(conffile_t cf, struct conffile_data *data,
         char *optionname, int option_type, void *option_ptr,
         int option_data, void *app_ptr, int app_data) 
{
  int *boolval = (int *)option_ptr;
  int cmdlineset = (int)option_data;

  if (cmdlineset)
    return 0;

  *boolval = data->boolval;
  return 0;
}

static int 
_cb_int(conffile_t cf, struct conffile_data *data,
        char *optionname, int option_type, void *option_ptr,
        int option_data, void *app_ptr, int app_data) 
{
  int *temp = (int *)option_ptr;
  int cmdlineset = (int)option_data;

  if (cmdlineset)
    return 0;

  *temp = data->intval; 
  return 0;
}

void 
ipmipower_config_conffile_parse(char *configfile) 
{
  int driver_type_flag,
    ipmi_version_flag, 
    hostname_flag, 
    hostnames_flag, 
    username_flag, 
    password_flag, 
    k_g_flag, 
    timeout_flag, 
    session_timeout_flag, 
    retry_timeout_flag,
    retransmission_timeout_flag, 
    authentication_type_flag, 
    cipher_suite_id_backwards_flag, 
    cipher_suite_id_flag, 
    privilege_flag, 
    privilege_level_flag, 
    workaround_flags_flag, 
    buffer_output_flag, 
    consolidate_output_flag,
    fanout_flag,
    eliminate_flag, 
    always_prefix_flag,
    on_if_off_flag, 
    wait_until_on_flag,
    wait_until_off_flag, 
    retry_wait_timeout_flag, 
    retransmission_wait_timeout_flag, 
    retry_backoff_count_flag, 
    retransmission_backoff_count_flag, 
    ping_interval_flag, 
    ping_timeout_flag,
    ping_packet_count_flag,
    ping_percent_flag, 
    ping_consec_count_flag;

  struct conffile_option options[] = 
    {
      {"driver-type", CONFFILE_OPTION_STRING, -1, _cb_driver_type,
       1, 0, &driver_type_flag, NULL, 0},
      /* ipmi-version maintained for backwards compatability */
      {"ipmi-version", CONFFILE_OPTION_STRING, -1, _cb_driver_type,
       1, 0, &ipmi_version_flag, NULL, 0},
      /* hostnames (plural) maintained for backwards compatability */
      {"hostnames", CONFFILE_OPTION_LIST_STRING, -1, _cb_hostname, 
       1, 0, &hostnames_flag, NULL, 0},
      {"hostname", CONFFILE_OPTION_LIST_STRING, -1, _cb_hostname, 
       1, 0, &hostname_flag, NULL, 0},
      {"username", CONFFILE_OPTION_STRING, -1, _cb_username,
       1, 0, &username_flag, NULL, 0},
      {"password", CONFFILE_OPTION_STRING, -1, _cb_password, 
       1, 0, &password_flag, NULL, 0},
      {"k_g", CONFFILE_OPTION_STRING, -1, _cb_k_g, 
       1, 0, &k_g_flag, NULL, 0},
      /* timeout maintained for backwards compatability */
      {"timeout", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &timeout_flag, &(conf->session_timeout_len), 
       conf->session_timeout_len_set_on_cmdline},
      {"session-timeout", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &session_timeout_flag, &(conf->session_timeout_len), 
       conf->session_timeout_len_set_on_cmdline},
      /* retry-timeout for backwards comptability */
      {"retry-timeout", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &retry_timeout_flag, &(conf->retransmission_timeout_len), 
       conf->retransmission_timeout_len_set_on_cmdline},
      {"retransmission-timeout", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &retransmission_timeout_flag, &(conf->retransmission_timeout_len), 
       conf->retransmission_timeout_len_set_on_cmdline},
      {"authentication-type", CONFFILE_OPTION_STRING, -1, _cb_authentication_type, 
       1, 0, &authentication_type_flag, NULL, 0},
      /* cipher suite id w/ underscores maintained for backwards compatability */
      {"cipher_suite_id", CONFFILE_OPTION_STRING, -1, _cb_int,
       1, 0, &cipher_suite_id_backwards_flag, NULL, 0},
      {"cipher-suite-id", CONFFILE_OPTION_STRING, -1, _cb_int,
       1, 0, &cipher_suite_id_flag, NULL, 0},
      /* "privilege" maintained for backwards compatability */
      {"privilege", CONFFILE_OPTION_STRING, -1, _cb_privilege_level, 
       1, 0, &privilege_flag, NULL, 0},
      {"privilege-level", CONFFILE_OPTION_STRING, -1, _cb_privilege_level, 
       1, 0, &privilege_level_flag, NULL, 0},
      {"workaround-flags", CONFFILE_OPTION_STRING, -1, _cb_workaround_flags,
       1, 0, &workaround_flags_flag, NULL, 0},
      {"buffer-output", CONFFILE_OPTION_BOOL, -1, _cb_bool,
       1, 0, &buffer_output_flag, NULL, 0},
      {"consolidate-output", CONFFILE_OPTION_BOOL, -1, _cb_bool,
       1, 0, &consolidate_output_flag, NULL, 0},
      {"fanout", CONFFILE_OPTION_INT, -1, _cb_int,
       1, 0, &fanout_flag, &(conf->fanout),
       conf->fanout_set_on_cmdline},
      {"eliminate", CONFFILE_OPTION_BOOL, -1, _cb_bool,
       1, 0, &eliminate_flag, NULL, 0},
      {"always_prefix", CONFFILE_OPTION_BOOL, -1, _cb_bool,
       1, 0, &always_prefix_flag, NULL, 0},
      {"on-if-off", CONFFILE_OPTION_BOOL, -1, _cb_bool,
       1, 0, &on_if_off_flag, &(conf->on_if_off), 
       conf->on_if_off_set_on_cmdline},
      {"wait-until-on", CONFFILE_OPTION_BOOL, -1, _cb_bool,
       1, 0, &wait_until_on_flag, &(conf->wait_until_on), 
       conf->wait_until_on_set_on_cmdline},
      {"wait-until-off", CONFFILE_OPTION_BOOL, -1, _cb_bool,
       1, 0, &wait_until_off_flag, &(conf->wait_until_off), 
       conf->wait_until_off_set_on_cmdline},
      /* retry-wait-timeout for backwards comptability */
      {"retry-wait-timeout", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &retry_wait_timeout_flag, &(conf->retransmission_wait_timeout_len), 
       conf->retransmission_wait_timeout_len_set_on_cmdline},
      {"retransmission-wait-timeout", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &retransmission_wait_timeout_flag, &(conf->retransmission_wait_timeout_len), 
       conf->retransmission_wait_timeout_len_set_on_cmdline},
      /* retry-backoff-count for backwards compatability */
      {"retry-backoff-count", CONFFILE_OPTION_INT, -1, _cb_int,
       1, 0, &retry_backoff_count_flag, &(conf->retransmission_backoff_count), 
       conf->retransmission_backoff_count_set_on_cmdline},
      {"retransmission-backoff-count", CONFFILE_OPTION_INT, -1, _cb_int,
       1, 0, &retransmission_backoff_count_flag, &(conf->retransmission_backoff_count), 
       conf->retransmission_backoff_count_set_on_cmdline},
      {"ping-interval", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &ping_interval_flag, &(conf->ping_interval_len), 
       conf->ping_interval_len_set_on_cmdline},
      {"ping-timeout", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &ping_timeout_flag, &(conf->ping_timeout_len), 
       conf->ping_timeout_len_set_on_cmdline},
      {"ping-packet-count", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &ping_packet_count_flag, &(conf->ping_packet_count), 
       conf->ping_packet_count_set_on_cmdline},
      {"ping-percent", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &ping_percent_flag, &(conf->ping_percent), 
       conf->ping_percent_set_on_cmdline},
      {"ping-consec-count", CONFFILE_OPTION_INT, -1, _cb_int, 
       1, 0, &ping_consec_count_flag, &(conf->ping_consec_count), 
       conf->ping_consec_count_set_on_cmdline},
    };
  conffile_t cf = NULL;
  char *conffile = NULL;
  int num;

  if ((cf = conffile_handle_create()) == NULL)
    ierr_exit("Config File Error: cannot create conffile handle");

  conffile = (strlen(configfile)) ? configfile : IPMIPOWER_CONFIG_FILE_DEFAULT;
  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, conffile, options, num, NULL, 0, 0) < 0) 
    {
      char errbuf[CONFFILE_MAX_ERRMSGLEN];
      
      /* Not an error if default file doesn't exist */ 
      if (!strlen(configfile) && conffile_errnum(cf) == CONFFILE_ERR_EXIST)
        goto done;
      
      if (conffile_errmsg(cf, errbuf, CONFFILE_MAX_ERRMSGLEN) < 0)
        ierr_exit("Config File Error: Cannot retrieve conffile error message");
      
      ierr_exit("Config File Error: %s", errbuf);
    }

  _config_common_checks("Config File Error");

 done:
  (void)conffile_handle_destroy(cf);
  return;
}

void 
ipmipower_config_check_values(void) 
{
  if (conf->driver_type == IPMI_DEVICE_LAN
      && conf->k_g_len)
    ierr_exit("Error: k_g is only used for IPMI 2.0");

  if (conf->driver_type == IPMI_DEVICE_LAN
      && strlen(conf->password) > IPMI_1_5_MAX_PASSWORD_LENGTH)
    ierr_exit("Error: password too long");

  if (conf->retransmission_timeout_len > conf->session_timeout_len)
    ierr_exit("Error: Session timeout length must be longer than retransmission  timeout length");
  
  if (conf->authentication_type == IPMI_AUTHENTICATION_TYPE_NONE
      && strlen(conf->password) > 0)
    ierr_exit("Error: password cannot be set for authentication type '%s'",
              IPMI_AUTHENTICATION_TYPE_NONE_STR);

  if (conf->fanout
      && (conf->fanout < PSTDOUT_FANOUT_MIN
          || conf->fanout > PSTDOUT_FANOUT_MAX))
    ierr_exit("Error: fanout invalid");

  if (conf->powercmd != POWER_CMD_NONE && conf->hosts == NULL)
    ierr_exit("Error: Must specify target hostname(s) in non-interactive mode");

  if (conf->ping_interval_len > conf->ping_timeout_len)
    ierr_exit("Error: Ping timeout interval length must be "
              "longer than ping interval length");

  if (conf->ping_consec_count > conf->ping_packet_count)
    ierr_exit("Error: Ping consec count must be larger than ping packet count");
}
