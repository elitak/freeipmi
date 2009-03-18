/*
   Copyright (C) 2003-2009 FreeIPMI Core Team

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

#ifndef _IPMI_EVENT_CMDS_H
#define _IPMI_EVENT_CMDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <freeipmi/fiid/fiid.h>

/* 
 * see freeipmi/templates/ for template definitions 
 */

extern fiid_template_t tmpl_cmd_set_event_receiver_rq;
extern fiid_template_t tmpl_cmd_set_event_receiver_rs;

extern fiid_template_t tmpl_cmd_get_event_receiver_rq;
extern fiid_template_t tmpl_cmd_get_event_receiver_rs;

int8_t fill_cmd_set_event_receiver (uint8_t event_receiver_slave_address,
                                    uint8_t event_receiver_lun,
                                    fiid_obj_t obj_cmd_rq);

int8_t fill_cmd_get_event_receiver (fiid_obj_t obj_cmd_rq);

#ifdef __cplusplus
}
#endif

#endif /* ipmi-event-cmds.h */
