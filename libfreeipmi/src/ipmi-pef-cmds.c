/*   ipmi-pef-cmds.c - IPMI System Event Log Commands */
   
/*   Copyright (C) 2003 - 2004 FreeIPMI Core Team */

/* This file is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 2, or (at your option) */
/* any later version. */

/* This file is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with GNU Emacs; see the file COPYING.  If not, write to */
/* the Free Software Foundation, Inc., 59 Temple Place - Suite 330, */
/* Boston, MA 02111-1307, USA. */

/* $Id: ipmi-pef-cmds.c,v 1.1 2004-10-26 22:16:23 itz Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "freeipmi.h"

fiid_template_t tmpl_get_pef_caps_rq =
  {
    {8, "cmd"}, 
    {0, ""}
  };

fiid_template_t tmpl_get_pef_caps_rs =
  {
    {8,  "cmd"}, 
    {8,  "comp_code"}, 
    {4,  "pef_version_major"}, 
    {4,  "pef_version_minor"}, 
    {1,  "get_pef_caps_alert_support"}, 
    {1,  "get_pef_caps_powerdown_support"}, 
    {1,  "get_pef_caps_reset_support"}, 
    {1,  "get_pef_caps_powercycle_support"},
    {1,  "get_pef_caps_oem_support"},
    {1,  "get_pef_caps_diag_interrupt_support"},
    {2,  "reserved"}, 
    {8,  "get_pef_caps_filter_table_entries"}, 
    {0,  ""}
  };

fiid_template_t tmpl_arm_pef_postpone_timer_rq =
  {
    {8, "cmd"},
    {8, "arm_pef_postpone_timer_countdown"},
    {0, ""}
  };

fiid_template_t tmpl_arm_pef_postpone_timer_rs =
  {
    {8, "cmd"},
    {8, "comp_code"},
    {8, "arm_pef_postpone_timer_countdown"},
    {0, ""}
  };

fiid_template_t tmpl_set_last_processed_event_rq =
  {
    {8, "cmd"},
    {1, "set_last_processed_event_which"},
    {7, "reserved"},
    {16, "set_last_processed_event_id"},
    {0, ""}
  };

fiid_template_t tmpl_set_last_processed_event_rs =
  {
    {8, "cmd"},
    {8, "comp_code"},
    {0, ""}
  };

fiid_template_t tmpl_get_last_processed_event_rq =
  {
    {8, "cmd"},
    {0, ""}
  };

fiid_template_t tmpl_get_last_processed_event_rs =
  {
    {8, "cmd"},
    {8, "comp_code"},
    {32, "get_last_processed_event_timestamp"},
    {16, "get_last_processed_event_sel_id"},
    {16, "get_last_processed_event_software_id"},
    {16, "get_last_processed_event_bmc_id"},
    {0, ""}
  };

fiid_template_t tmpl_pet_ack_rq =
  {
    {8, "cmd"},
    {16, "pet_ack_sequence_number"},
    {32, "pet_ack_timestamp"},
    {8, "pet_ack_source_type"},
    {8, "pet_ack_sensor_device"},
    {8, "pet_ack_sensor_number"},
    {24, "pet_ack_event_data"},
    {0, ""}
  };

fiid_template_t tmpl_pet_ack_rs =
  {
    {8, "cmd"},
    {8, "comp_code"},
    {0, ""}
  };

static int8_t 
fill_kcs_get_pef_caps (fiid_obj_t obj_data_rq)
{
  FIID_OBJ_SET (obj_data_rq, 
		tmpl_get_pef_caps_rq, 
		"cmd", 
                IPMI_CMD_GET_PEF_CAPS);
  return 0;
}

static int8_t
fill_kcs_arm_pef_postpone_timer (fiid_obj_t obj_data_rq, u_int8_t countdown)
{
  FIID_OBJ_SET (obj_data_rq,
                tmpl_arm_pef_postpone_timer_rq,
                "cmd",
                IPMI_CMD_ARM_PEF_POSTPONE_TIMER);
  FIID_OBJ_SET (obj_data_rq,
                tmpl_arm_pef_postpone_timer_rq,
                "arm_pef_postpone_timer_countdown",
                countdown);
  return 0;
}

static int8_t
fill_kcs_set_last_processed_event (fiid_obj_t obj_data_rq, which_event_t which, u_int16_t id)
{
  FIID_OBJ_SET (obj_data_rq,
                tmpl_set_last_processed_event_rq,
                "cmd",
                IPMI_CMD_SET_LAST_PROCESSED_EVENT_ID);
  FIID_OBJ_SET (obj_data_rq,
                tmpl_set_last_processed_event_rq,
                "set_last_processed_event_which",
                which);
  FIID_OBJ_SET (obj_data_rq,
                tmpl_set_last_processed_event_rq,
                "set_last_processed_event_id",
                id);
  return 0;                
}

static int8_t
fill_kcs_get_last_proessed_event (fiid_obj_t obj_data_rq)
{
  FIID_OBJ_SET (obj_data_rq,
                tmpl_get_last_processed_event_rq,
                "cmd",
                IPMI_CMD_GET_LAST_PROCESSED_EVENT_ID);
  return 0;
}

static int8_t
fill_kcs_pet_ack (fiid_obj_t obj_data_rq, u_int16_t sequence_number, u_int32_t timestamp,
                  u_int8_t source_type, u_int8_t sensor_device, u_int8_t sensor_number,
                  u_int32_t event_data)
{
  FIID_OBJ_SET (obj_data_rq,
                tmpl_pet_ack_rq,
                "cmd",
                IPMI_CMD_PET_ACKNOWLEDGE);
  FIID_OBJ_SET (obj_data_rq,
                tmpl_pet_ack_rq,
                "pet_ack_sequence_number",
                sequence_number);
  FIID_OBJ_SET (obj_data_rq,
                tmpl_pet_ack_rq,
                "pet_ack_timestamp",
                timestamp);
  FIID_OBJ_SET (obj_data_rq,
                tmpl_pet_ack_rq,
                "pet_ack_source_type",
                source_type);
  FIID_OBJ_SET (obj_data_rq,
                tmpl_pet_ack_rq,
                "pet_ack_sensor_device",
                sensor_device);
  FIID_OBJ_SET (obj_data_rq,
                tmpl_pet_ack_rq,
                "pet_ack_sensor_number",
                sensor_number);
  FIID_OBJ_SET (obj_data_rq,
                tmpl_pet_ack_rq,
                "pet_ack_event_data",
                event_data);
  return 0;  
}

int8_t 
ipmi_kcs_get_pef_caps (u_int16_t sms_io_base, fiid_obj_t obj_data_rs)
{
  fiid_obj_t obj_data_rq; 
  int8_t status;
  
  obj_data_rq = fiid_obj_alloc (tmpl_get_sel_info_rq);
  fill_kcs_get_pef_caps (obj_data_rq);
  status = ipmi_kcs_cmd (sms_io_base, IPMI_BMC_IPMB_LUN_BMC, IPMI_NET_FN_SENSOR_EVENT_RQ,
			 obj_data_rq, tmpl_get_pef_caps_rq, 
			 obj_data_rs, tmpl_get_pef_caps_rs);
  free (obj_data_rq);
  return status;
}

int8_t
ipmi_kcs_arm_pef_postpone_timer (u_int16_t sms_io_base, fiid_obj_t obj_data_rs, u_int8_t countdown)
{
  fiid_obj_t obj_data_rq;
  int8_t status;

  obj_data_rq = fiid_obj_alloc (tmpl_arm_pef_postpone_timer_rq);
  fill_kcs_arm_pef_postpone_timer (obj_data_rq, countdown);
  status = ipmi_kcs_cmd (sms_io_base, IPMI_BMC_IPMB_LUN_BMC, IPMI_NET_FN_SENSOR_EVENT_RQ,
			 obj_data_rq, tmpl_arm_pef_postpone_timer_rq, 
			 obj_data_rs, tmpl_arm_pef_postpone_timer_rs);
  free (obj_data_rq);
  return status;
}  

int8_t
ipmi_kcs_set_last_processed_event (u_int16_t sms_io_base, fiid_obj_t obj_data_rs, which_event_t which, u_int16_t id)
{
  fiid_obj_t obj_data_rq;
  int8_t status;

  obj_data_rq = fiid_obj_alloc (tmpl_set_last_processed_event_rq);
  fill_kcs_set_last_processed_event (obj_data_rq, which, id);
  status = ipmi_kcs_cmd (sms_io_base, IPMI_BMC_IPMB_LUN_BMC, IPMI_NET_FN_SENSOR_EVENT_RQ,
			 obj_data_rq, tmpl_set_last_processed_event_rq, 
			 obj_data_rs, tmpl_set_last_processed_event_rs);
  free (obj_data_rq);
  return status;
}

int8_t
ipmi_kcs_get_last_processed_event (u_int16_t sms_io_base, fiid_obj_t obj_data_rs)
{
  fiid_obj_t obj_data_rq;
  int8_t status;

  obj_data_rq = fiid_obj_alloc (tmpl_get_last_processed_event_rq);
  fill_kcs_get_last_proessed_event (obj_data_rq);
  status = ipmi_kcs_cmd (sms_io_base, IPMI_BMC_IPMB_LUN_BMC, IPMI_NET_FN_SENSOR_EVENT_RQ,
			 obj_data_rq, tmpl_get_last_processed_event_rq, 
			 obj_data_rs, tmpl_get_last_processed_event_rs);
  free (obj_data_rq);
  return status;
}

int8_t
ipmi_pet_ack (u_int16_t sms_io_base, fiid_obj_t obj_data_rs, u_int16_t sequence_number,
              u_int32_t timestamp, u_int8_t source_type, u_int8_t sensor_device,
              u_int8_t sensor_number, u_int32_t event_data)
{
  fiid_obj_t obj_data_rq;
  int8_t status;

  obj_data_rq = fiid_obj_alloc (tmpl_pet_ack_rq);
  fill_kcs_pet_ack (obj_data_rq, sequence_number, timestamp, source_type,
                    sensor_device, sensor_number, event_data);
  status = ipmi_kcs_cmd (sms_io_base, IPMI_BMC_IPMB_LUN_BMC, IPMI_NET_FN_SENSOR_EVENT_RQ,
			 obj_data_rq, tmpl_pet_ack_rq,
			 obj_data_rs, tmpl_pet_ack_rs);
  free (obj_data_rq);
  return status;
}
