/*
  Copyright (C) 2007-2009 FreeIPMI Core Team

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <assert.h>

#include "pef-config.h"
#include "pef-config-map.h"
#include "pef-config-utils.h"
#include "pef-config-validate.h"

#include "freeipmi-portability.h"
#include "pstdout.h"
#include "tool-fiid-util.h"

/* convenience struct */
struct alert_policy_table {
  uint8_t policy_type;
  uint8_t policy_enabled;
  uint8_t policy_number;
  uint8_t destination_selector;
  uint8_t channel_number;
  uint8_t alert_string_set_selector;
  uint8_t event_specific_alert_string;
};

static config_err_t
_get_alert_policy_table (struct pef_config_state_data *state_data,
                         const char *section_name,
                         struct alert_policy_table *apt)
{
  fiid_obj_t obj_cmd_rs = NULL;
  uint64_t val;
  config_err_t rv = CONFIG_ERR_FATAL_ERROR;
  uint8_t alert_policy_entry_number;

  assert (state_data);
  assert (section_name);
  assert (apt);

  alert_policy_entry_number = atoi (section_name + strlen ("Alert_Policy_"));

  if (!(obj_cmd_rs = fiid_obj_create (tmpl_cmd_get_pef_configuration_parameters_alert_policy_table_rs)))
    {
      pstdout_fprintf (state_data->pstate,
                       stderr,
                       "fiid_obj_create: %s\n",
                       strerror (errno));
      goto cleanup;
    }

  if (ipmi_cmd_get_pef_configuration_parameters_alert_policy_table (state_data->ipmi_ctx,
                                                                    IPMI_GET_PEF_PARAMETER,
                                                                    alert_policy_entry_number,
                                                                    BLOCK_SELECTOR,
                                                                    obj_cmd_rs) < 0)
    {
      if (state_data->prog_data->args->config_args.common.debug)
        pstdout_fprintf (state_data->pstate,
                         stderr,
                         "ipmi_cmd_get_pef_configuration_parameters_alert_policy_table: %s\n",
                         ipmi_ctx_errormsg (state_data->ipmi_ctx));
      if (!IPMI_ERRNUM_IS_FATAL_ERROR (state_data->ipmi_ctx))
        rv = CONFIG_ERR_NON_FATAL_ERROR;
      goto cleanup;
    }

#if 0
  TOOL_FIID_OBJ_GET (obj_cmd_rs, "alert_policy_entry_number", &val);
#endif
  TOOL_FIID_OBJ_GET (obj_cmd_rs, "policy_number.policy_type", &val);
  apt->policy_type = val;

  TOOL_FIID_OBJ_GET (obj_cmd_rs, "policy_number.enabled", &val);
  apt->policy_enabled = val;

  TOOL_FIID_OBJ_GET (obj_cmd_rs, "policy_number.policy_number", &val);
  apt->policy_number = val;

  TOOL_FIID_OBJ_GET (obj_cmd_rs, "channel_destination.destination_selector", &val);
  apt->destination_selector = val;

  TOOL_FIID_OBJ_GET (obj_cmd_rs, "channel_destination.channel_number", &val);
  apt->channel_number = val;

  TOOL_FIID_OBJ_GET (obj_cmd_rs, "alert_string_key.alert_string_set_selector", &val);
  apt->alert_string_set_selector = val;

  TOOL_FIID_OBJ_GET (obj_cmd_rs, "alert_string_key.event_specific_alert_string", &val);
  apt->event_specific_alert_string = val;

  rv = CONFIG_ERR_SUCCESS;
 cleanup:
  fiid_obj_destroy (obj_cmd_rs);
  return (rv);
}

static config_err_t
_set_alert_policy_table (struct pef_config_state_data *state_data,
                         const char *section_name,
                         struct alert_policy_table *apt)
{
  fiid_obj_t obj_cmd_rs = NULL;
  config_err_t rv = CONFIG_ERR_FATAL_ERROR;
  uint8_t alert_policy_entry_number;

  assert (state_data);
  assert (section_name);
  assert (apt);

  alert_policy_entry_number = atoi (section_name + strlen ("Alert_Policy_"));

  if (!(obj_cmd_rs = fiid_obj_create (tmpl_cmd_set_pef_configuration_parameters_rs)))
    {
      pstdout_fprintf (state_data->pstate,
                       stderr,
                       "fiid_obj_create: %s\n",
                       strerror (errno));
      goto cleanup;
    }

  if (ipmi_cmd_set_pef_configuration_parameters_alert_policy_table (state_data->ipmi_ctx,
                                                                    alert_policy_entry_number,
                                                                    apt->policy_type,
                                                                    apt->policy_enabled,
                                                                    apt->policy_number,
                                                                    apt->destination_selector,
                                                                    apt->channel_number,
                                                                    apt->alert_string_set_selector,
                                                                    apt->event_specific_alert_string,
                                                                    obj_cmd_rs) < 0)
    {
      if (state_data->prog_data->args->config_args.common.debug)
        pstdout_fprintf (state_data->pstate,
                         stderr,
                         "ipmi_cmd_set_pef_configuration_parameters_alert_policy_table: %s\n",
                         ipmi_ctx_errormsg (state_data->ipmi_ctx));
      if (!IPMI_ERRNUM_IS_FATAL_ERROR (state_data->ipmi_ctx))
        rv = CONFIG_ERR_NON_FATAL_ERROR;
      goto cleanup;
    }

  rv = CONFIG_ERR_SUCCESS;
 cleanup:
  fiid_obj_destroy (obj_cmd_rs);
  return (rv);
}

static config_err_t
policy_type_checkout (const char *section_name,
                      struct config_keyvalue *kv,
                      void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  if (config_section_update_keyvalue_output (state_data->pstate,
                                             kv,
                                             policy_type_string (apt.policy_type)) < 0)
    return (CONFIG_ERR_FATAL_ERROR);

  return (CONFIG_ERR_SUCCESS);
}

static config_err_t
policy_type_commit (const char *section_name,
                    const struct config_keyvalue *kv,
                    void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  apt.policy_type = policy_type_number (kv->value_input);

  return (_set_alert_policy_table (state_data,
                                   section_name,
                                   &apt));
}

static config_err_t
policy_enabled_checkout (const char *section_name,
                         struct config_keyvalue *kv,
                         void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  if (config_section_update_keyvalue_output (state_data->pstate,
                                             kv,
                                             apt.policy_enabled ? "Yes" : "No") < 0)
    return (CONFIG_ERR_FATAL_ERROR);

  return (CONFIG_ERR_SUCCESS);
}

static config_err_t
policy_enabled_commit (const char *section_name,
                       const struct config_keyvalue *kv,
                       void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  apt.policy_enabled = same (kv->value_input, "yes");

  return (_set_alert_policy_table (state_data,
                                   section_name,
                                   &apt));
}

static config_err_t
policy_number_checkout (const char *section_name,
                        struct config_keyvalue *kv,
                        void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  if (config_section_update_keyvalue_output_int (state_data->pstate,
                                                 kv,
                                                 apt.policy_number) < 0)
    return (CONFIG_ERR_FATAL_ERROR);

  return (CONFIG_ERR_SUCCESS);
}

static config_err_t
policy_number_commit (const char *section_name,
                      const struct config_keyvalue *kv,
                      void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  apt.policy_number = atoi (kv->value_input);

  return (_set_alert_policy_table (state_data,
                                   section_name,
                                   &apt));
}

static config_err_t
destination_selector_checkout (const char *section_name,
                               struct config_keyvalue *kv,
                               void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  if (config_section_update_keyvalue_output_int (state_data->pstate,
                                                 kv,
                                                 apt.destination_selector) < 0)
    return (CONFIG_ERR_FATAL_ERROR);

  return (CONFIG_ERR_SUCCESS);
}

static config_err_t
destination_selector_commit (const char *section_name,
                             const struct config_keyvalue *kv,
                             void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  apt.destination_selector = atoi (kv->value_input);

  return (_set_alert_policy_table (state_data,
                                   section_name,
                                   &apt));
}

static config_err_t
channel_number_checkout (const char *section_name,
                         struct config_keyvalue *kv,
                         void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  if (config_section_update_keyvalue_output_int (state_data->pstate,
                                                 kv,
                                                 apt.channel_number) < 0)
    return (CONFIG_ERR_FATAL_ERROR);

  return (CONFIG_ERR_SUCCESS);
}

static config_err_t
channel_number_commit (const char *section_name,
                       const struct config_keyvalue *kv,
                       void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  apt.channel_number = atoi (kv->value_input);

  return (_set_alert_policy_table (state_data,
                                   section_name,
                                   &apt));
}

static config_err_t
alert_string_set_selector_checkout (const char *section_name,
                                    struct config_keyvalue *kv,
                                    void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  if (config_section_update_keyvalue_output_int (state_data->pstate,
                                                 kv,
                                                 apt.alert_string_set_selector) < 0)
    return (CONFIG_ERR_FATAL_ERROR);

  return (CONFIG_ERR_SUCCESS);
}

static config_err_t
alert_string_set_selector_commit (const char *section_name,
                                  const struct config_keyvalue *kv,
                                  void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  apt.alert_string_set_selector = atoi (kv->value_input);

  return (_set_alert_policy_table (state_data,
                                   section_name,
                                   &apt));
}

static config_err_t
event_specific_alert_string_checkout (const char *section_name,
                                      struct config_keyvalue *kv,
                                      void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  if (config_section_update_keyvalue_output (state_data->pstate,
                                             kv,
                                             apt.event_specific_alert_string ? "Yes" : "No") < 0)
    return (CONFIG_ERR_FATAL_ERROR);

  return (CONFIG_ERR_SUCCESS);
}

static config_err_t
event_specific_alert_string_commit (const char *section_name,
                                    const struct config_keyvalue *kv,
                                    void *arg)
{
  pef_config_state_data_t *state_data = (pef_config_state_data_t *)arg;
  struct alert_policy_table apt;
  config_err_t ret;

  if ((ret = _get_alert_policy_table (state_data,
                                      section_name,
                                      &apt)) != CONFIG_ERR_SUCCESS)
    return (ret);

  apt.event_specific_alert_string = same (kv->value_input, "yes");

  return (_set_alert_policy_table (state_data,
                                   section_name,
                                   &apt));
}

struct config_section *
pef_config_alert_policy_table_section_get (pef_config_state_data_t *state_data, int num)
{
  struct config_section *section = NULL;
  uint8_t lan_channel_number;
  char *strp = NULL;
  config_err_t ret;
  char buf[CONFIG_MAX_SECTION_NAME_LEN];

  if (num <= 0)
    {
      pstdout_fprintf (state_data->pstate,
                       stderr,
                       "Invalid Num = %d\n",
                       num);
      return (NULL);
    }

  snprintf (buf, CONFIG_MAX_SECTION_NAME_LEN, "Alert_Policy_%d", num);

  if (!(section = config_section_create (state_data->pstate,
                                         buf,
                                         NULL,
                                         NULL,
                                         0,
                                         NULL,
                                         NULL)))
    goto cleanup;

  if (config_section_add_key (state_data->pstate,
                              section,
                              "Policy_Type",
                              "Possible values: Always_Send_To_This_Destination/Proceed_To_Next_Entry/Do_Not_Proceed_Any_More_Entries/Proceed_To_Next_Entry_Different_Channel/Proceed_To_Next_Entry_Different_Destination_Type",
                              0,
                              policy_type_checkout,
                              policy_type_commit,
                              policy_type_validate) < 0)
    goto cleanup;

  if (config_section_add_key (state_data->pstate,
                              section,
                              "Policy_Enabled",
                              "Possible values: Yes/No",
                              0,
                              policy_enabled_checkout,
                              policy_enabled_commit,
                              config_yes_no_validate) < 0)
    goto cleanup;

  if (config_section_add_key (state_data->pstate,
                              section,
                              "Policy_Number",
                              "Give a valid number",
                              0,
                              policy_number_checkout,
                              policy_number_commit,
                              config_number_range_four_bits) < 0)
    goto cleanup;

  if (config_section_add_key (state_data->pstate,
                              section,
                              "Destination_Selector",
                              "Give a valid number",
                              0,
                              destination_selector_checkout,
                              destination_selector_commit,
                              config_number_range_four_bits) < 0)
    goto cleanup;

  ret = get_lan_channel_number(state_data, &lan_channel_number);
  if (ret == CONFIG_ERR_SUCCESS)
    {
      if (asprintf (&strp,
                    "Give a valid number (LAN = %u)",
                    lan_channel_number) < 0)
        {
          if (!strp)
            {
              pstdout_perror (state_data->pstate,
                              "asprintf");
              goto cleanup;
            }
        }
    }
  else
    {
      if (!(strp = strdup ("Give a valid number")))
        {
          pstdout_perror (state_data->pstate,
                          "strdup");
          goto cleanup;
        }
    }

  if (config_section_add_key (state_data->pstate,
                              section,
                              "Channel_Number",
                              strp,
                              0,
                              channel_number_checkout,
                              channel_number_commit,
                              config_number_range_four_bits) < 0)
    goto cleanup;

  if (config_section_add_key (state_data->pstate,
                              section,
                              "Alert_String_Set_Selector",
                              "Give a valid number",
                              0,
                              alert_string_set_selector_checkout,
                              alert_string_set_selector_commit,
                              config_number_range_seven_bits) < 0)
    goto cleanup;

  if (config_section_add_key (state_data->pstate,
                              section,
                              "Event_Specific_Alert_String",
                              "Possible values: Yes/No",
                              0,
                              event_specific_alert_string_checkout,
                              event_specific_alert_string_commit,
                              config_yes_no_validate) < 0)
    goto cleanup;

  if (strp)
    free (strp);
  return (section);

 cleanup:
  if (strp)
    free (strp);
  if (section)
    config_section_destroy (state_data->pstate, section);
  return (NULL);
}

