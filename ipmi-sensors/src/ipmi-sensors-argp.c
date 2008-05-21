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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <argp.h>
#include <errno.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <assert.h>

#include "tool-cmdline-common.h"
#include "tool-sensor-common.h"

#include "ipmi-sensors.h"
#include "ipmi-sensors-argp.h"
#include "ipmi-sensors-util.h"

#include "freeipmi-portability.h"

const char *argp_program_version = 
  "ipmi-sensors - " PACKAGE_VERSION "\n"
  "Copyright (C) 2003-2008 FreeIPMI Core Team\n"
  "This program is free software; you may redistribute it under the terms of\n"
  "the GNU General Public License.  This program has absolutely no warranty.";

const char *argp_program_bug_address = 
  "<" PACKAGE_BUGREPORT ">";

static char cmdline_doc[] = 
  "ipmi-sensors - displays IPMI sensor information";

static char cmdline_args_doc[] = "";

static struct argp_option cmdline_options[] = 
  {
    ARGP_COMMON_OPTIONS_DRIVER,
    ARGP_COMMON_OPTIONS_INBAND,
    ARGP_COMMON_OPTIONS_OUTOFBAND,
    ARGP_COMMON_OPTIONS_AUTHENTICATION_TYPE,
    ARGP_COMMON_OPTIONS_CIPHER_SUITE_ID,
    ARGP_COMMON_OPTIONS_PRIVILEGE_LEVEL_USER,
    ARGP_COMMON_OPTIONS_WORKAROUND_FLAGS,
    ARGP_COMMON_SDR_OPTIONS,
    ARGP_COMMON_HOSTRANGED_OPTIONS,
    ARGP_COMMON_OPTIONS_DEBUG,
    {"verbose",        VERBOSE_KEY,        0, 0, 
     "Increase verbosity in output.  May be specified multiple times.", 30}, 
    {"quiet-readings", QUIET_READINGS_KEY,  0, 0,
     "Do not output sensor readings or thresholds on simple output.", 31},
    {"sdr-info",       SDR_INFO_KEY,       0, 0, 
     "Show sendor data repository (SDR) information.", 32}, 
    {"list-groups",    LIST_GROUPS_KEY,    0, 0, 
     "List sensor groups.", 33}, 
    /* maintain "group" for backwards compatability */
    {"group",          GROUP_KEY,        "GROUP-NAME", OPTION_HIDDEN, 
     "Show sensors belonging to a specific group.", 34}, 
    {"groups",         GROUPS_KEY,       "GROUP-NAME", 0, 
     "Show sensors belonging to a specific group.", 35}, 
    {"sensors",        SENSORS_LIST_KEY, "SENSORS-LIST", 0, 
     "Show sensors by record id.  Accepts space or comma separated lists", 36}, 
    { 0 }
  };

static error_t cmdline_parse (int key, char *arg, struct argp_state *state);

static struct argp cmdline_argp = { cmdline_options,
                                    cmdline_parse, 
                                    cmdline_args_doc, 
                                    cmdline_doc };

static error_t 
cmdline_parse (int key, char *arg, struct argp_state *state)
{
  struct ipmi_sensors_arguments *cmd_args = state->input;
  char *ptr;
  char *tok;
  error_t ret;
  
  switch (key)
    {
    case VERBOSE_KEY:
      cmd_args->verbose = 1;
      cmd_args->verbose_count++;
      break;
    case QUIET_READINGS_KEY:
      cmd_args->quiet_readings = 1;
      break;
    case SDR_INFO_KEY:
      cmd_args->sdr_info = 1;
      break;
    case LIST_GROUPS_KEY:
      cmd_args->list_groups = 1;
      break;
    /* maintain "group" for backwards compatability */
    case GROUP_KEY:
      cmd_args->groups_list_wanted = 1;
      strncpy(cmd_args->groups_list[cmd_args->groups_list_length], 
              arg, 
              IPMI_SENSORS_MAX_GROUPS_STRING_LENGTH);
      cmd_args->groups_list_length++;
      break;
    case GROUPS_KEY:
      cmd_args->groups_list_wanted = 1;
      tok = strtok(arg, " ,");
      while (tok && cmd_args->groups_list_length < IPMI_SENSORS_MAX_GROUPS)
        {
          strncpy(cmd_args->groups_list[cmd_args->groups_list_length],
                  tok,
                  IPMI_SENSORS_MAX_GROUPS_STRING_LENGTH);
          cmd_args->groups_list_length++;
          tok = strtok(NULL, " ,");
        }
      break;
    case SENSORS_LIST_KEY:
      cmd_args->sensors_list_wanted = 1;
      tok = strtok(arg, " ,");
      while (tok && cmd_args->sensors_list_length < IPMI_SENSORS_MAX_RECORD_IDS)
        {
          unsigned int n = strtoul(tok, &ptr, 10);
          if (ptr != (tok + strlen(tok)))
            {
              fprintf (stderr, "invalid sensor record id\n");
              exit(1);
            }
          cmd_args->sensors_list[cmd_args->sensors_list_length] = n;
          cmd_args->sensors_list_length++;
          tok = strtok(NULL, " ,");
        }
      break;
    case ARGP_KEY_ARG:
      /* Too many arguments. */
      argp_usage (state);
      break;
    case ARGP_KEY_END:
      break;
    default:
      ret = common_parse_opt (key, arg, state, &(cmd_args->common));
      if (ret == ARGP_ERR_UNKNOWN)
        ret = sdr_parse_opt (key, arg, state, &(cmd_args->sdr));
      if (ret == ARGP_ERR_UNKNOWN)
        ret = hostrange_parse_opt (key, arg, state, &(cmd_args->hostrange));
      return ret;
    }
  
  return 0;
}

void
_ipmi_sensors_args_validate (struct ipmi_sensors_arguments *cmd_args)
{
  if (cmd_args->groups_list_length)
    {
      int i;

      for (i = 0; i < cmd_args->groups_list_length; i++)
        {
          int j = 0;
          int found = 0;
          
          while (ipmi_sensor_types[j])
            {
              char sdr_group_name_subst[IPMI_SENSORS_MAX_GROUPS_STRING_LENGTH];
              
              strcpy(sdr_group_name_subst, ipmi_sensor_types[j]);
              str_replace_char (sdr_group_name_subst, ' ', '_');
              
              if (!strcasecmp (cmd_args->groups_list[i], ipmi_sensor_types[j])
                  || !strcasecmp (cmd_args->groups_list[i], sdr_group_name_subst))
                {
                  found++;
                  break;
                }
              j++;
            }

          if (!found)
            {
              fprintf(stderr, "invalid sensor group '%s'\n", cmd_args->groups_list[i]);
              exit(1);
            }
            
        }
    }
}

void 
ipmi_sensors_argp_parse (int argc, char **argv, struct ipmi_sensors_arguments *cmd_args)
{
  int i;

  init_common_cmd_args_user (&(cmd_args->common));
  init_sdr_cmd_args (&(cmd_args->sdr));
  init_hostrange_cmd_args (&(cmd_args->hostrange));
  cmd_args->verbose = 0;
  cmd_args->verbose_count = 0;
  cmd_args->quiet_readings = 0;
  cmd_args->sdr_info = 0;
  cmd_args->list_groups = 0;
  cmd_args->groups_list_wanted = 0;
  for (i = 0; i < IPMI_SENSORS_MAX_GROUPS; i++)
    memset(cmd_args->groups_list[i],
           '\0',
           IPMI_SENSORS_MAX_GROUPS_STRING_LENGTH+1);
  cmd_args->groups_list_length = 0;
  cmd_args->sensors_list_wanted = 0;
  memset(cmd_args->sensors_list, 
         '\0', 
         sizeof(unsigned int)*IPMI_SENSORS_MAX_RECORD_IDS);
  cmd_args->sensors_list_length = 0;
  
  argp_parse (&cmdline_argp, argc, argv, ARGP_IN_ORDER, NULL, cmd_args);
  verify_common_cmd_args (&(cmd_args->common));
  verify_sdr_cmd_args (&(cmd_args->sdr));
  verify_hostrange_cmd_args (&(cmd_args->hostrange));
  _ipmi_sensors_args_validate (cmd_args);
}


