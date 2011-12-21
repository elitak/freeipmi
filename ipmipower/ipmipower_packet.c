/*****************************************************************************\
 *  $Id: ipmipower_packet.c,v 1.127 2010-06-11 16:29:34 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2012 Lawrence Livermore National Security, LLC.
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
 *  Free Software Foundation; either version 3 of the License, or (at your
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
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <errno.h>
#include <assert.h>
#include <stdint.h>

#include "ipmipower_packet.h"
#include "ipmipower_error.h"
#include "ipmipower_util.h"

#include "freeipmi-portability.h"
#include "debug-util.h"

extern struct ipmipower_arguments cmd_args;

fiid_field_t *
ipmipower_packet_cmd_template (ipmipower_powercmd_t ip, packet_type_t pkt)
{
  assert (ip);
  assert (PACKET_TYPE_VALID_PKT (pkt));

  if (pkt == AUTHENTICATION_CAPABILITIES_REQ)
    return (&tmpl_cmd_get_channel_authentication_capabilities_rq[0]);
  else if (pkt == AUTHENTICATION_CAPABILITIES_RES)
    return (&tmpl_cmd_get_channel_authentication_capabilities_rs[0]);
  else if (pkt == GET_SESSION_CHALLENGE_REQ)
    return (&tmpl_cmd_get_session_challenge_rq[0]);
  else if (pkt == GET_SESSION_CHALLENGE_RES)
    return (&tmpl_cmd_get_session_challenge_rs[0]);
  else if (pkt == ACTIVATE_SESSION_REQ)
    return (&tmpl_cmd_activate_session_rq[0]);
  else if (pkt == ACTIVATE_SESSION_RES)
    return (&tmpl_cmd_activate_session_rs[0]);
  else if (pkt == OPEN_SESSION_REQ)
    return (&tmpl_rmcpplus_open_session_request[0]);
  else if (pkt == OPEN_SESSION_RES)
    return (&tmpl_rmcpplus_open_session_response[0]);
  else if (pkt == RAKP_MESSAGE_1_REQ)
    return (&tmpl_rmcpplus_rakp_message_1[0]);
  else if (pkt == RAKP_MESSAGE_2_RES)
    return (&tmpl_rmcpplus_rakp_message_2[0]);
  else if (pkt == RAKP_MESSAGE_3_REQ)
    return (&tmpl_rmcpplus_rakp_message_3[0]);
  else if (pkt == RAKP_MESSAGE_4_RES)
    return (&tmpl_rmcpplus_rakp_message_4[0]);
  else if (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ)
    return (&tmpl_cmd_set_session_privilege_level_rq[0]);
  else if (pkt == SET_SESSION_PRIVILEGE_LEVEL_RES)
    return (&tmpl_cmd_set_session_privilege_level_rs[0]);
  else if (pkt == GET_CHASSIS_STATUS_REQ)
    return (&tmpl_cmd_get_chassis_status_rq[0]);
  else if (pkt == GET_CHASSIS_STATUS_RES)
    return (&tmpl_cmd_get_chassis_status_rs[0]);
  else if (pkt == CHASSIS_CONTROL_REQ)
    return (&tmpl_cmd_chassis_control_rq[0]);
  else if (pkt == CHASSIS_CONTROL_RES)
    return (&tmpl_cmd_chassis_control_rs[0]);
  else if (pkt == CHASSIS_IDENTIFY_REQ)
    return (&tmpl_cmd_chassis_identify_rq[0]);
  else if (pkt == CHASSIS_IDENTIFY_RES)
    return (&tmpl_cmd_chassis_identify_rs[0]);
  else if (pkt == CLOSE_SESSION_REQ)
    return (&tmpl_cmd_close_session_rq[0]);
  else if (pkt == CLOSE_SESSION_RES)
    return (&tmpl_cmd_close_session_rs[0]);
  else
    {
      IPMIPOWER_ERROR (("ipmipower_packet_cmd_template: invalid packet type: %d",
                        ip->protocol_state));
      exit (1);
    }

  return (NULL);                  /* NOT REACHED */
}

fiid_obj_t
ipmipower_packet_cmd_obj (ipmipower_powercmd_t ip, packet_type_t pkt)
{
  assert (ip);
  assert (PACKET_TYPE_VALID_PKT (pkt));

  if (pkt == AUTHENTICATION_CAPABILITIES_REQ)
    return (ip->obj_authentication_capabilities_req);
  else if (pkt == AUTHENTICATION_CAPABILITIES_RES)
    return (ip->obj_authentication_capabilities_res);
  else if (pkt == GET_SESSION_CHALLENGE_REQ)
    return (ip->obj_get_session_challenge_req);
  else if (pkt == GET_SESSION_CHALLENGE_RES)
    return (ip->obj_get_session_challenge_res);
  else if (pkt == ACTIVATE_SESSION_REQ)
    return (ip->obj_activate_session_req);
  else if (pkt == ACTIVATE_SESSION_RES)
    return (ip->obj_activate_session_res);
  else if (pkt == OPEN_SESSION_REQ)
    return (ip->obj_open_session_req);
  else if (pkt == OPEN_SESSION_RES)
    return (ip->obj_open_session_res);
  else if (pkt == RAKP_MESSAGE_1_REQ)
    return (ip->obj_rakp_message_1_req);
  else if (pkt == RAKP_MESSAGE_2_RES)
    return (ip->obj_rakp_message_2_res);
  else if (pkt == RAKP_MESSAGE_3_REQ)
    return (ip->obj_rakp_message_3_req);
  else if (pkt == RAKP_MESSAGE_4_RES)
    return (ip->obj_rakp_message_4_res);
  else if (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ)
    return (ip->obj_set_session_privilege_level_req);
  else if (pkt == SET_SESSION_PRIVILEGE_LEVEL_RES)
    return (ip->obj_set_session_privilege_level_res);
  else if (pkt == GET_CHASSIS_STATUS_REQ)
    return (ip->obj_get_chassis_status_req);
  else if (pkt == GET_CHASSIS_STATUS_RES)
    return (ip->obj_get_chassis_status_res);
  else if (pkt == CHASSIS_CONTROL_REQ)
    return (ip->obj_chassis_control_req);
  else if (pkt == CHASSIS_CONTROL_RES)
    return (ip->obj_chassis_control_res);
  else if (pkt == CHASSIS_IDENTIFY_REQ)
    return (ip->obj_chassis_identify_req);
  else if (pkt == CHASSIS_IDENTIFY_RES)
    return (ip->obj_chassis_identify_res);
  else if (pkt == CLOSE_SESSION_REQ)
    return (ip->obj_close_session_req);
  else if (pkt == CLOSE_SESSION_RES)
    return (ip->obj_close_session_res);
  else
    {
      IPMIPOWER_ERROR (("ipmipower_packet_cmd_obj: invalid packet type: %d",
                        ip->protocol_state));
      exit (1);
    }

  return (NULL);                  /* NOT REACHED */
}

void
ipmipower_packet_dump (ipmipower_powercmd_t ip, packet_type_t pkt,
                       const void *buf,
                       unsigned int buflen)
{
  assert (ip);
  assert (PACKET_TYPE_VALID_PKT (pkt));
  assert (buf);

  if (cmd_args.common.debug)
    {
      fiid_field_t *tmpl_lan_msg_hdr;
      char hdrbuf[DEBUG_UTIL_HDR_BUFLEN];
      uint8_t packet_type;
      uint8_t packet_direction;
      const char *str_cmd = NULL;

      if (cmd_args.common.driver_type == IPMI_DEVICE_LAN)
        packet_type = DEBUG_UTIL_TYPE_IPMI_1_5;
      else
        packet_type = DEBUG_UTIL_TYPE_IPMI_2_0;

      if (pkt == AUTHENTICATION_CAPABILITIES_REQ
          || pkt == AUTHENTICATION_CAPABILITIES_RES)
        str_cmd = ipmi_cmd_str (IPMI_NET_FN_APP_RQ, IPMI_CMD_GET_CHANNEL_AUTHENTICATION_CAPABILITIES);
      else if (pkt == GET_SESSION_CHALLENGE_REQ
               || pkt == GET_SESSION_CHALLENGE_RES)
        str_cmd = ipmi_cmd_str (IPMI_NET_FN_APP_RQ, IPMI_CMD_GET_SESSION_CHALLENGE);
      else if (pkt == ACTIVATE_SESSION_REQ
               || pkt == ACTIVATE_SESSION_RES)
        str_cmd = ipmi_cmd_str (IPMI_NET_FN_APP_RQ, IPMI_CMD_ACTIVATE_SESSION);
      else if (pkt == OPEN_SESSION_REQ
               || pkt == OPEN_SESSION_RES)
        str_cmd = DEBUG_UTIL_OPEN_SESSION_STR;
      else if (pkt == RAKP_MESSAGE_1_REQ)
        str_cmd = DEBUG_UTIL_RAKP_1_STR;
      else if (pkt == RAKP_MESSAGE_2_RES)
        str_cmd = DEBUG_UTIL_RAKP_2_STR;
      else if (pkt == RAKP_MESSAGE_3_REQ)
        str_cmd = DEBUG_UTIL_RAKP_3_STR;
      else if (pkt == RAKP_MESSAGE_4_RES)
        str_cmd = DEBUG_UTIL_RAKP_4_STR;
      else if (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ
               || pkt == SET_SESSION_PRIVILEGE_LEVEL_RES)
        str_cmd = ipmi_cmd_str (IPMI_NET_FN_APP_RQ, IPMI_CMD_SET_SESSION_PRIVILEGE_LEVEL);
      else if (pkt == GET_CHASSIS_STATUS_REQ
               || pkt == GET_CHASSIS_STATUS_RES)
        str_cmd = ipmi_cmd_str (IPMI_NET_FN_CHASSIS_RQ, IPMI_CMD_GET_CHASSIS_STATUS);
      else if (pkt == CHASSIS_CONTROL_REQ
               || pkt == CHASSIS_CONTROL_RES)
        str_cmd = ipmi_cmd_str (IPMI_NET_FN_CHASSIS_RQ, IPMI_CMD_CHASSIS_CONTROL);
      else if (pkt == CHASSIS_IDENTIFY_REQ
               || pkt == CHASSIS_IDENTIFY_RES)
        str_cmd = ipmi_cmd_str (IPMI_NET_FN_CHASSIS_RQ, IPMI_CMD_CHASSIS_IDENTIFY);
      else if (pkt == CLOSE_SESSION_REQ
               || pkt == CLOSE_SESSION_RES)
        str_cmd = ipmi_cmd_str (IPMI_NET_FN_APP_RQ, IPMI_CMD_CLOSE_SESSION);

      if (pkt & PACKET_TYPE_REQ_MASK)
        packet_direction = DEBUG_UTIL_DIRECTION_REQUEST;
      else
        packet_direction = DEBUG_UTIL_DIRECTION_RESPONSE;

      debug_hdr_str (packet_type,
                     packet_direction,
		     DEBUG_UTIL_FLAGS_DEFAULT,
                     str_cmd,
                     hdrbuf,
                     DEBUG_UTIL_HDR_BUFLEN);

      if (pkt & PACKET_TYPE_REQ_MASK)
        tmpl_lan_msg_hdr = &tmpl_lan_msg_hdr_rq[0];
      else
        tmpl_lan_msg_hdr = &tmpl_lan_msg_hdr_rs[0];

      if (pkt == OPEN_SESSION_REQ
          || pkt == OPEN_SESSION_RES
          || pkt == RAKP_MESSAGE_1_REQ
          || pkt == RAKP_MESSAGE_2_RES
          || pkt == RAKP_MESSAGE_3_REQ
          || pkt == RAKP_MESSAGE_4_RES)
        {
          if (ipmi_dump_rmcpplus_packet (STDERR_FILENO,
                                         ip->ic->hostname,
                                         hdrbuf,
                                         NULL,
                                         IPMI_AUTHENTICATION_ALGORITHM_RAKP_NONE,
                                         IPMI_INTEGRITY_ALGORITHM_NONE,
                                         IPMI_CONFIDENTIALITY_ALGORITHM_NONE,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         buf,
                                         buflen,
                                         tmpl_lan_msg_hdr,
                                         ipmipower_packet_cmd_template (ip, pkt)) < 0)
            {
              IPMIPOWER_ERROR (("ipmi_dump_rmcpplus_packet: %s", strerror (errno)));
              exit (1);
            }
        }
      else if (cmd_args.common.driver_type == IPMI_DEVICE_LAN_2_0
               && (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ
                   || pkt == SET_SESSION_PRIVILEGE_LEVEL_RES
                   || pkt == GET_CHASSIS_STATUS_REQ
                   || pkt == GET_CHASSIS_STATUS_RES
                   || pkt == CHASSIS_CONTROL_REQ
                   || pkt == CHASSIS_CONTROL_RES
                   || pkt == CHASSIS_IDENTIFY_REQ
                   || pkt == CHASSIS_IDENTIFY_RES
                   || pkt == CLOSE_SESSION_REQ
                   || pkt == CLOSE_SESSION_RES))
        {
          if (ipmi_dump_rmcpplus_packet (STDERR_FILENO,
                                         ip->ic->hostname,
                                         hdrbuf,
                                         NULL,
                                         ip->authentication_algorithm,
                                         ip->integrity_algorithm,
                                         ip->confidentiality_algorithm,
                                         ip->integrity_key_ptr,
                                         ip->integrity_key_len,
                                         ip->confidentiality_key_ptr,
                                         ip->confidentiality_key_len,
                                         buf,
                                         buflen,
                                         tmpl_lan_msg_hdr,
                                         ipmipower_packet_cmd_template (ip, pkt)) < 0)
            {
              IPMIPOWER_ERROR (("ipmi_dump_rmcpplus_packet: %s", strerror (errno)));
              exit (1);
            }
        }
      else /* IPMI 1.5 pkt */
        {
          if (ipmi_dump_lan_packet (STDERR_FILENO,
                                    ip->ic->hostname,
                                    hdrbuf,
                                    NULL,
                                    buf,
                                    buflen,
                                    tmpl_lan_msg_hdr,
                                    ipmipower_packet_cmd_template (ip, pkt)) < 0)
            {
              IPMIPOWER_ERROR (("ipmi_dump_lan_packet: %s", strerror (errno)));
              exit (1);
            }
        }
    }
}

int
ipmipower_packet_store (ipmipower_powercmd_t ip,
                        packet_type_t pkt,
                        const void *buf,
                        unsigned int buflen)
{
  fiid_obj_t obj;
  int rv = -1;

  assert (ip);
  assert (buf);
  assert (buflen);
  assert (PACKET_TYPE_VALID_RES (pkt));

  obj = ipmipower_packet_cmd_obj (ip, pkt);

  if (fiid_obj_clear (ip->obj_rmcp_hdr_res) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_rmcp_hdr_res)));
      exit (1);
    }
  if (fiid_obj_clear (ip->obj_lan_session_hdr_res) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_lan_session_hdr_res)));
      exit (1);
    }
  if (fiid_obj_clear (ip->obj_lan_msg_hdr_res) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_lan_msg_hdr_res)));
      exit (1);
    }
  if (fiid_obj_clear (ip->obj_lan_msg_trlr_res) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_lan_msg_trlr_res)));
      exit (1);
    }

  if (cmd_args.common.driver_type == IPMI_DEVICE_LAN_2_0)
    {
      if (fiid_obj_clear (ip->obj_rmcpplus_session_hdr_res) < 0)
        {
          IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_rmcpplus_session_hdr_res)));
          exit (1);
        }
      if (fiid_obj_clear (ip->obj_rmcpplus_payload_res) < 0)
        {
          IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_rmcpplus_payload_res)));
          exit (1);
        }
      if (fiid_obj_clear (ip->obj_rmcpplus_session_trlr_res) < 0)
        {
          IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_rmcpplus_session_trlr_res)));
          exit (1);
        }
    }

  if (fiid_obj_clear (obj) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (obj)));
      exit (1);
    }

  if (pkt == AUTHENTICATION_CAPABILITIES_RES
      || pkt == GET_SESSION_CHALLENGE_RES
      || pkt == ACTIVATE_SESSION_RES
      || cmd_args.common.driver_type == IPMI_DEVICE_LAN)
    {
      if ((rv = unassemble_ipmi_lan_pkt (buf,
                                         buflen,
                                         ip->obj_rmcp_hdr_res,
                                         ip->obj_lan_session_hdr_res,
                                         ip->obj_lan_msg_hdr_res,
                                         obj,
                                         ip->obj_lan_msg_trlr_res,
					 IPMI_INTERFACE_FLAGS_DEFAULT)) < 0)
        {
          IPMIPOWER_ERROR (("unassemble_ipmi_lan_pkt: %s", strerror (errno)));
          exit (1);
        }
    }
  else
    {
      if (pkt == OPEN_SESSION_RES
          || pkt == RAKP_MESSAGE_2_RES
          || pkt == RAKP_MESSAGE_4_RES)
        {
          if ((rv = unassemble_ipmi_rmcpplus_pkt (IPMI_AUTHENTICATION_ALGORITHM_RAKP_NONE,
                                                  IPMI_INTEGRITY_ALGORITHM_NONE,
                                                  IPMI_CONFIDENTIALITY_ALGORITHM_NONE,
                                                  NULL,
                                                  0,
                                                  NULL,
                                                  0,
                                                  buf,
                                                  buflen,
                                                  ip->obj_rmcp_hdr_res,
                                                  ip->obj_rmcpplus_session_hdr_res,
                                                  ip->obj_rmcpplus_payload_res,
                                                  ip->obj_lan_msg_hdr_res,
                                                  obj,
                                                  ip->obj_lan_msg_trlr_res,
                                                  ip->obj_rmcpplus_session_trlr_res,
						  IPMI_INTERFACE_FLAGS_DEFAULT)) < 0)
            {
              IPMIPOWER_ERROR (("unassemble_ipmi_rmcpplus_pkt: %s", strerror (errno)));
              exit (1);
            }
        }
      else
        {
          if ((rv = unassemble_ipmi_rmcpplus_pkt (ip->authentication_algorithm,
                                                  ip->integrity_algorithm,
                                                  ip->confidentiality_algorithm,
                                                  ip->integrity_key_ptr,
                                                  ip->integrity_key_len,
                                                  ip->confidentiality_key_ptr,
                                                  ip->confidentiality_key_len,
                                                  buf,
                                                  buflen,
                                                  ip->obj_rmcp_hdr_res,
                                                  ip->obj_rmcpplus_session_hdr_res,
                                                  ip->obj_rmcpplus_payload_res,
                                                  ip->obj_lan_msg_hdr_res,
                                                  obj,
                                                  ip->obj_lan_msg_trlr_res,
                                                  ip->obj_rmcpplus_session_trlr_res,
						  IPMI_INTERFACE_FLAGS_DEFAULT)) < 0)
            {
              IPMIPOWER_ERROR (("unassemble_ipmi_rmcpplus_pkt: %s", strerror (errno)));
              exit (1);
            }
        }
    }

  return (rv);
}

static int
_ipmi_1_5_packet_create (ipmipower_powercmd_t ip,
                         packet_type_t pkt,
                         uint8_t authentication_type,
                         uint32_t inbound_sequence_number,
                         uint32_t session_id,
                         void *authentication_code_data,
                         unsigned int authentication_code_data_len,
                         uint8_t net_fn,
                         fiid_obj_t obj_cmd_req,
                         void *buf,
                         unsigned int buflen)
{
  int len;

  assert (ip);
  assert (PACKET_TYPE_VALID_REQ (pkt));
  assert (fiid_obj_valid (obj_cmd_req));
  assert (buf);
  assert (buflen);

  if (fiid_obj_clear (ip->obj_rmcp_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_rmcp_hdr_req)));
      exit (1);
    }
  if (fiid_obj_clear (ip->obj_lan_session_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_lan_session_hdr_req)));
      exit (1);
    }
  if (fiid_obj_clear (ip->obj_lan_msg_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_lan_msg_hdr_req)));
      exit (1);
    }

  if (fill_rmcp_hdr_ipmi (ip->obj_rmcp_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fill_rmcp_hdr_ipmi: %s", strerror (errno)));
      exit (1);
    }

  if (fill_lan_session_hdr (authentication_type,
                            inbound_sequence_number,
                            session_id,
                            ip->obj_lan_session_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fill_lan_session_hdr: %s", strerror (errno)));
      exit (1);
    }

  if (fill_lan_msg_hdr (IPMI_SLAVE_ADDRESS_BMC,
                        net_fn,
                        IPMI_BMC_IPMB_LUN_BMC,
                        (ip->ic->ipmi_requester_sequence_number_counter % (IPMI_LAN_REQUESTER_SEQUENCE_NUMBER_MAX + 1)),
                        ip->obj_lan_msg_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fill_lan_msg_hdr: %s", strerror (errno)));
      exit (1);
    }

  if ((len = assemble_ipmi_lan_pkt (ip->obj_rmcp_hdr_req,
                                    ip->obj_lan_session_hdr_req,
                                    ip->obj_lan_msg_hdr_req,
                                    obj_cmd_req,
                                    authentication_code_data,
                                    authentication_code_data_len,
                                    buf,
                                    buflen,
				    IPMI_INTERFACE_FLAGS_DEFAULT)) < 0)
    {
      IPMIPOWER_ERROR (("assemble_ipmi_lan_pkt: %s", strerror (errno)));
      exit (1);
    }

  return (len);
}

static int
_ipmi_2_0_packet_create (ipmipower_powercmd_t ip,
                         packet_type_t pkt,
                         uint8_t payload_type,
                         uint8_t payload_authenticated,
                         uint8_t payload_encrypted,
                         uint32_t session_id,
                         uint32_t session_sequence_number,
                         void *authentication_code_data,
                         unsigned int authentication_code_data_len,
                         uint8_t net_fn,
                         uint8_t authentication_algorithm,
                         uint8_t integrity_algorithm,
                         uint8_t confidentiality_algorithm,
                         void *integrity_key,
                         unsigned int integrity_key_len,
                         void *confidentiality_key,
                         unsigned int confidentiality_key_len,
                         fiid_obj_t obj_cmd_req,
                         void *buf,
                         unsigned int buflen)
{
  int len;

  assert (ip);
  assert (PACKET_TYPE_VALID_REQ (pkt));
  assert (fiid_obj_valid (obj_cmd_req));
  assert (buf);
  assert (buflen);

  if (fiid_obj_clear (ip->obj_rmcp_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_rmcp_hdr_req)));
      exit (1);
    }
  if (fiid_obj_clear (ip->obj_lan_msg_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_lan_msg_hdr_req)));
      exit (1);
    }
  if (fiid_obj_clear (ip->obj_rmcpplus_session_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_rmcpplus_session_hdr_req)));
      exit (1);
    }
  if (fiid_obj_clear (ip->obj_rmcpplus_session_trlr_req) < 0)
    {
      IPMIPOWER_ERROR (("fiid_obj_clear: %s", fiid_obj_errormsg (ip->obj_rmcpplus_session_trlr_req)));
      exit (1);
    }

  if (fill_rmcp_hdr_ipmi (ip->obj_rmcp_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fill_rmcp_hdr_ipmi: %s", strerror (errno)));
      exit (1);
    }

  if (fill_rmcpplus_session_hdr (payload_type,
                                 payload_authenticated,
                                 payload_encrypted,
                                 0, /* oem_iana */
                                 0, /* oem_payload_id */
                                 session_id,
                                 session_sequence_number,
                                 ip->obj_rmcpplus_session_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fill_rmcpplus_session_hdr: %s", strerror (errno)));
      exit (1);
    }

  if (fill_lan_msg_hdr (IPMI_SLAVE_ADDRESS_BMC,
                        net_fn,
                        IPMI_BMC_IPMB_LUN_BMC,
                        (ip->ic->ipmi_requester_sequence_number_counter % (IPMI_LAN_REQUESTER_SEQUENCE_NUMBER_MAX + 1)),
                        ip->obj_lan_msg_hdr_req) < 0)
    {
      IPMIPOWER_ERROR (("fill_lan_msg_hdr: %s", strerror (errno)));
      exit (1);
    }

  if (fill_rmcpplus_session_trlr (ip->obj_rmcpplus_session_trlr_req) < 0)
    {
      IPMIPOWER_ERROR (("fill_rmcpplus_session_trlr: %s", strerror (errno)));
      exit (1);
    }

  if ((len = assemble_ipmi_rmcpplus_pkt (authentication_algorithm,
                                         integrity_algorithm,
                                         confidentiality_algorithm,
                                         integrity_key,
                                         integrity_key_len,
                                         confidentiality_key,
                                         confidentiality_key_len,
                                         authentication_code_data,
                                         authentication_code_data_len,
                                         ip->obj_rmcp_hdr_req,
                                         ip->obj_rmcpplus_session_hdr_req,
                                         ip->obj_lan_msg_hdr_req,
                                         obj_cmd_req,
                                         ip->obj_rmcpplus_session_trlr_req,
                                         buf,
                                         buflen,
					 IPMI_INTERFACE_FLAGS_DEFAULT)) < 0)
    {
      IPMIPOWER_ERROR (("assemble_ipmi_rmcpplus_pkt: %s", strerror (errno)));
      exit (1);
    }

  return (len);
}

int
ipmipower_packet_create (ipmipower_powercmd_t ip,
                         packet_type_t pkt,
                         void *buf,
                         unsigned int buflen)
{
  char *username = NULL;
  char *password = NULL;
  void *integrity_key = NULL;
  void *confidentiality_key = NULL;
  char username_buf[IPMI_MAX_USER_NAME_LENGTH+1];
  unsigned int username_len;
  uint32_t session_id, managed_system_session_id = 0;
  uint32_t sequence_number = 0;
  unsigned int integrity_key_len = 0;
  unsigned int confidentiality_key_len = 0;
  uint8_t authentication_type = 0;
  uint8_t net_fn = 0;
  uint8_t payload_authenticated = 0;
  uint8_t payload_encrypted = 0;
  uint8_t payload_type = 0;
  uint8_t authentication_algorithm = 0;
  uint8_t integrity_algorithm = 0;
  uint8_t confidentiality_algorithm = 0;
  fiid_obj_t obj_cmd_req = NULL;
  uint64_t val;
  int rv = 0;

  assert (ip);
  assert (PACKET_TYPE_VALID_REQ (pkt));
  assert (buf);
  assert (buflen);

  if (pkt == GET_SESSION_CHALLENGE_REQ
      || pkt == RAKP_MESSAGE_1_REQ
      || pkt == RAKP_MESSAGE_3_REQ)
    {
      username = cmd_args.common.username;

      /* IPMI Workaround (achu)
       *
       * Discovered on SE7520AF2 with Intel Server Management Module
       * (Professional Edition)
       *
       * The username must be padded despite explicitly not being
       * allowed.  "No Null characters (00h) are allowed in the name".
       * Table 13-11 in the IPMI 2.0 spec.
       */
      if (pkt == RAKP_MESSAGE_1_REQ
          && (cmd_args.common.workaround_flags_outofband_2_0 & IPMI_PARSE_WORKAROUND_FLAGS_OUTOFBAND_2_0_INTEL_2_0_SESSION))
        {
          memset (username_buf, '\0', IPMI_MAX_USER_NAME_LENGTH+1);
          if (username)
            strcpy (username_buf, username);
          username = username_buf;
          username_len = IPMI_MAX_USER_NAME_LENGTH;
        }
      else
        username_len = (username) ? strlen (username) : 0;
    }
  else
    {
      username = NULL;
      username_len = 0;
    }

  /* Calculate Password */
  if (pkt == ACTIVATE_SESSION_REQ
      || pkt == OPEN_SESSION_REQ
      || pkt == RAKP_MESSAGE_1_REQ
      || pkt == RAKP_MESSAGE_3_REQ
      || pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ
      || pkt == GET_CHASSIS_STATUS_REQ
      || pkt == CHASSIS_CONTROL_REQ
      || pkt == CHASSIS_IDENTIFY_REQ
      || pkt == CLOSE_SESSION_REQ)
    password = cmd_args.common.password;
  else
    password = NULL;

  /* Calculate Session ID */
  if (pkt == ACTIVATE_SESSION_REQ)
    {
      if (FIID_OBJ_GET (ip->obj_get_session_challenge_res,
                        "temp_session_id",
                        &val) < 0)
        {
          IPMIPOWER_ERROR (("FIID_OBJ_GET: 'temp_session_id': %s",
                            fiid_obj_errormsg (ip->obj_get_session_challenge_res)));
          exit (1);
        }

      session_id = val;
    }
  else if (cmd_args.common.driver_type == IPMI_DEVICE_LAN
           && (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ
               || pkt == GET_CHASSIS_STATUS_REQ
               || pkt == CHASSIS_CONTROL_REQ
               || pkt == CHASSIS_IDENTIFY_REQ
               || pkt == CLOSE_SESSION_REQ))
    {
      if (FIID_OBJ_GET (ip->obj_activate_session_res,
                        "session_id",
                        &val) < 0)
        {
          IPMIPOWER_ERROR (("FIID_OBJ_GET: 'session_id': %s",
                            fiid_obj_errormsg (ip->obj_activate_session_res)));
          exit (1);
        }
      session_id = val;
    }
  else if (cmd_args.common.driver_type == IPMI_DEVICE_LAN_2_0
           && (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ
               || pkt == GET_CHASSIS_STATUS_REQ
               || pkt == CHASSIS_CONTROL_REQ
               || pkt == CHASSIS_IDENTIFY_REQ
               || pkt == CLOSE_SESSION_REQ))
    {
      if (FIID_OBJ_GET (ip->obj_open_session_res,
                        "managed_system_session_id",
                        &val) < 0)
        {
          IPMIPOWER_ERROR (("FIID_OBJ_GET: 'managed_system_session_id': %s",
                            fiid_obj_errormsg (ip->obj_open_session_res)));
          exit (1);
        }
      session_id = val;
    }
  else
    session_id = 0;

  /* Calculate Sequence Number */
  if (cmd_args.common.driver_type == IPMI_DEVICE_LAN
      && (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ
          || pkt == GET_CHASSIS_STATUS_REQ
          || pkt == CHASSIS_CONTROL_REQ
          || pkt == CHASSIS_IDENTIFY_REQ
          || pkt == CLOSE_SESSION_REQ))
    {
      uint32_t initial_inbound_sequence_number;

      if (FIID_OBJ_GET (ip->obj_activate_session_res,
                        "initial_inbound_sequence_number",
                        &val) < 0)
        {
          IPMIPOWER_ERROR (("FIID_OBJ_GET: 'initial_inbound_sequence_number': %s",
                            fiid_obj_errormsg (ip->obj_activate_session_res)));
          exit (1);
        }
      initial_inbound_sequence_number = val;

      sequence_number = initial_inbound_sequence_number + ip->session_inbound_count;
    }
  else if (cmd_args.common.driver_type == IPMI_DEVICE_LAN_2_0
           && (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ
               || pkt == GET_CHASSIS_STATUS_REQ
               || pkt == CHASSIS_CONTROL_REQ
               || pkt == CHASSIS_IDENTIFY_REQ
               || pkt == CLOSE_SESSION_REQ))
    sequence_number = ip->session_sequence_number;
  else
    sequence_number = 0;

  /* Calculate Network Function */
  if (pkt == GET_CHASSIS_STATUS_REQ
      || pkt == CHASSIS_CONTROL_REQ
      || pkt == CHASSIS_IDENTIFY_REQ)
    net_fn = IPMI_NET_FN_CHASSIS_RQ;
  else
    net_fn = IPMI_NET_FN_APP_RQ;

  /* Calculate Authentication Type */
  if (pkt == ACTIVATE_SESSION_REQ)
    authentication_type = cmd_args.common.authentication_type;
  else if (cmd_args.common.driver_type == IPMI_DEVICE_LAN
           && (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ
               || pkt == GET_CHASSIS_STATUS_REQ
               || pkt == CHASSIS_CONTROL_REQ
               || pkt == CHASSIS_IDENTIFY_REQ
               || pkt == CLOSE_SESSION_REQ))
    {
      if (!ip->permsgauth_enabled)
        authentication_type = IPMI_AUTHENTICATION_TYPE_NONE;
      else
        authentication_type = cmd_args.common.authentication_type;

      if (authentication_type == IPMI_AUTHENTICATION_TYPE_NONE)
        password = NULL;
    }
  else
    authentication_type = IPMI_AUTHENTICATION_TYPE_NONE;

  if (cmd_args.common.driver_type == IPMI_DEVICE_LAN_2_0)
    {
      /* Calculate Payload Type */
      if (pkt == OPEN_SESSION_REQ)
        payload_type = IPMI_PAYLOAD_TYPE_RMCPPLUS_OPEN_SESSION_REQUEST;
      else if (pkt == RAKP_MESSAGE_1_REQ)
        payload_type = IPMI_PAYLOAD_TYPE_RAKP_MESSAGE_1;
      else if (pkt == RAKP_MESSAGE_3_REQ)
        payload_type = IPMI_PAYLOAD_TYPE_RAKP_MESSAGE_3;
      else
        payload_type = IPMI_PAYLOAD_TYPE_IPMI;

      /* achu: "session_id" above is for the session headers.  This is
       * for the RAKP session setup protocol.  The values will be
       * different.
       */
      if (pkt == RAKP_MESSAGE_1_REQ
          || pkt == RAKP_MESSAGE_3_REQ)
        {
          if (FIID_OBJ_GET (ip->obj_open_session_res,
                            "managed_system_session_id",
                            &val) < 0)
            {
              IPMIPOWER_ERROR (("FIID_OBJ_GET: 'managed_system_session_id': %s",
                                fiid_obj_errormsg (ip->obj_open_session_res)));
              exit (1);
            }
          managed_system_session_id = val;
        }

      /* Setup authentication/integrity/confidentiality keys */
      if (pkt == OPEN_SESSION_REQ
          || pkt == RAKP_MESSAGE_1_REQ
          || pkt == RAKP_MESSAGE_3_REQ)
        {
          authentication_algorithm = IPMI_AUTHENTICATION_ALGORITHM_RAKP_NONE;
          integrity_algorithm = IPMI_INTEGRITY_ALGORITHM_NONE;
          confidentiality_algorithm = IPMI_CONFIDENTIALITY_ALGORITHM_NONE;
          integrity_key = NULL;
          integrity_key_len = 0;
          confidentiality_key = NULL;
          confidentiality_key_len = 0;
        }
      else
        {
          authentication_algorithm = ip->authentication_algorithm;
          integrity_algorithm = ip->integrity_algorithm;
          confidentiality_algorithm = ip->confidentiality_algorithm;
          integrity_key = ip->integrity_key_ptr;
          integrity_key_len = ip->integrity_key_len;
          confidentiality_key = ip->confidentiality_key_ptr;
          confidentiality_key_len = ip->confidentiality_key_len;
        }

      /* Calculate Payload Authenticated */
      if (pkt == OPEN_SESSION_REQ
          || pkt == RAKP_MESSAGE_1_REQ
          || pkt == RAKP_MESSAGE_3_REQ
          || integrity_algorithm == IPMI_INTEGRITY_ALGORITHM_NONE)
        payload_authenticated = IPMI_PAYLOAD_FLAG_UNAUTHENTICATED;
      else
        payload_authenticated = IPMI_PAYLOAD_FLAG_AUTHENTICATED;

      /* Calculate Payload Encrypted */
      if (pkt == OPEN_SESSION_REQ
          || pkt == RAKP_MESSAGE_1_REQ
          || pkt == RAKP_MESSAGE_3_REQ
          || confidentiality_algorithm == IPMI_CONFIDENTIALITY_ALGORITHM_NONE)
        payload_encrypted = IPMI_PAYLOAD_FLAG_UNENCRYPTED;
      else
        payload_encrypted = IPMI_PAYLOAD_FLAG_ENCRYPTED;
    }

  /* Calculate/Fill Command Object */
  if (pkt == AUTHENTICATION_CAPABILITIES_REQ)
    {
      uint8_t get_ipmi_v20_extended_data;
      
      if (cmd_args.common.driver_type == IPMI_DEVICE_LAN_2_0)
        get_ipmi_v20_extended_data = IPMI_GET_IPMI_V20_EXTENDED_DATA;
      else
        get_ipmi_v20_extended_data = IPMI_GET_IPMI_V15_DATA;
      
      if (fill_cmd_get_channel_authentication_capabilities (IPMI_CHANNEL_NUMBER_CURRENT_CHANNEL,
                                                            cmd_args.common.privilege_level,
                                                            get_ipmi_v20_extended_data,
                                                            ip->obj_authentication_capabilities_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_cmd_get_channel_authentication_capabilities: %s",
                            strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_authentication_capabilities_req;
    }
  else if (pkt == GET_SESSION_CHALLENGE_REQ)
    {
      /* Note: The session_authentication_type is none, this authentication type may be different.
       */
      if (fill_cmd_get_session_challenge (cmd_args.common.authentication_type,
                                          username,
                                          username_len,
                                          ip->obj_get_session_challenge_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_cmd_get_session_challenge: %s", strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_get_session_challenge_req;
    }
  else if (pkt == ACTIVATE_SESSION_REQ)
    {
      uint8_t challenge_string[IPMI_CHALLENGE_STRING_LENGTH];
      int challenge_string_len;

      if ((challenge_string_len = fiid_obj_get_data (ip->obj_get_session_challenge_res,
                                                     "challenge_string",
                                                     challenge_string,
                                                     IPMI_CHALLENGE_STRING_LENGTH)) < 0)
        {
          IPMIPOWER_ERROR (("fiid_obj_get_data: 'challenge_string': %s",
                            fiid_obj_errormsg (ip->obj_get_session_challenge_res)));
          exit (1);
        }

      if (!challenge_string_len)
        {
          IPMIPOWER_ERROR (("host = %s; p = %d; empty challenge string",
                            ip->ic->hostname, ip->protocol_state));
          exit (1);
        }

      if (fill_cmd_activate_session (authentication_type,
                                     cmd_args.common.privilege_level,
                                     challenge_string,
                                     challenge_string_len,
                                     IPMIPOWER_LAN_INITIAL_OUTBOUND_SEQUENCE_NUMBER,
                                     ip->obj_activate_session_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_cmd_activate_session: %s", strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_activate_session_req;
    }
  else if (pkt == OPEN_SESSION_REQ)
    {
      if (fill_rmcpplus_open_session (ip->initial_message_tag + ip->message_tag_count,
                                      ip->requested_maximum_privilege_level,
                                      ip->remote_console_session_id,
                                      ip->authentication_algorithm,
                                      ip->integrity_algorithm,
                                      ip->confidentiality_algorithm,
                                      ip->obj_open_session_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_rmcpplus_open_session: %s", strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_open_session_req;
    }
  else if (pkt == RAKP_MESSAGE_1_REQ)
    {
      if (fill_rmcpplus_rakp_message_1 (ip->initial_message_tag + ip->message_tag_count,
                                        managed_system_session_id,
                                        ip->remote_console_random_number,
                                        IPMI_REMOTE_CONSOLE_RANDOM_NUMBER_LENGTH,
                                        cmd_args.common.privilege_level,
                                        ip->name_only_lookup,
                                        username,
                                        username_len,
                                        ip->obj_rakp_message_1_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_rmcpplus_rakp_message_1: %s", strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_rakp_message_1_req;
    }
  else if (pkt == RAKP_MESSAGE_3_REQ)
    {
      uint8_t managed_system_random_number[IPMI_MANAGED_SYSTEM_RANDOM_NUMBER_LENGTH];
      int managed_system_random_number_len;
      uint8_t key_exchange_authentication_code[IPMI_MAX_KEY_EXCHANGE_AUTHENTICATION_CODE_LENGTH];
      int key_exchange_authentication_code_len;
      uint8_t name_only_lookup;
      unsigned int password_len;

      if ((managed_system_random_number_len = fiid_obj_get_data (ip->obj_rakp_message_2_res,
                                                                 "managed_system_random_number",
                                                                 managed_system_random_number,
                                                                 IPMI_MANAGED_SYSTEM_RANDOM_NUMBER_LENGTH)) < 0)
        {
          IPMIPOWER_ERROR (("fiid_obj_get_data: 'managed_system_random_number': %s",
                            fiid_obj_errormsg (ip->obj_rakp_message_2_res)));
          exit (1);
        }

      /* IPMI Workaround (achu)
       *
       * Discovered on SE7520AF2 with Intel Server Management Module
       * (Professional Edition)
       *
       * For some reason we have to create this key with the name only
       * lookup turned off.  I was skeptical about this actually being
       * a bug until I saw that the ipmitool folks implemented the
       * same workaround.
       */

      if (cmd_args.common.workaround_flags_outofband_2_0 & IPMI_PARSE_WORKAROUND_FLAGS_OUTOFBAND_2_0_INTEL_2_0_SESSION)
        name_only_lookup = IPMI_USER_NAME_PRIVILEGE_LOOKUP;
      else
        name_only_lookup = ip->name_only_lookup;

      password_len = (password) ? strlen (password) : 0;

      /* IPMI Workaround (achu)
       *
       * Discovered on SE7520AF2 with Intel Server Management Module
       * (Professional Edition)
       *
       * When the authentication algorithm is HMAC-MD5-128 and the
       * password is greater than 16 bytes, the Intel BMC truncates the
       * password to 16 bytes when generating keys, hashes, etc.  So we
       * have to do the same when generating keys, hashes, etc.
       */
      if ((cmd_args.common.workaround_flags_outofband_2_0 & IPMI_PARSE_WORKAROUND_FLAGS_OUTOFBAND_2_0_INTEL_2_0_SESSION)
          && ip->authentication_algorithm == IPMI_AUTHENTICATION_ALGORITHM_RAKP_HMAC_MD5
          && password_len > IPMI_1_5_MAX_PASSWORD_LENGTH)
        password_len = IPMI_1_5_MAX_PASSWORD_LENGTH;

      if ((key_exchange_authentication_code_len = ipmi_calculate_rakp_3_key_exchange_authentication_code (ip->authentication_algorithm,
                                                                                                          password,
                                                                                                          password_len,
                                                                                                          managed_system_random_number,
                                                                                                          managed_system_random_number_len,
                                                                                                          ip->remote_console_session_id,
                                                                                                          name_only_lookup,
                                                                                                          cmd_args.common.privilege_level,
                                                                                                          username,
                                                                                                          username_len,
                                                                                                          key_exchange_authentication_code,
                                                                                                          IPMI_MAX_KEY_EXCHANGE_AUTHENTICATION_CODE_LENGTH)) < 0)
        {
          IPMIPOWER_ERROR (("ipmi_calculate_rakp_3_key_exchange_authentication_code: %s",
                            strerror (errno)));
          exit (1);
        }

      if (fill_rmcpplus_rakp_message_3 (ip->initial_message_tag + ip->message_tag_count,
                                        RMCPPLUS_STATUS_NO_ERRORS,
                                        managed_system_session_id,
                                        key_exchange_authentication_code,
                                        key_exchange_authentication_code_len,
                                        ip->obj_rakp_message_3_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_rmcpplus_rakp_message_3: %s", strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_rakp_message_3_req;
    }
  else if (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ)
    {
      if (fill_cmd_set_session_privilege_level (cmd_args.common.privilege_level,
                                                ip->obj_set_session_privilege_level_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_cmd_set_session_privilege_level: %s", strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_set_session_privilege_level_req;
    }
  else if (pkt == GET_CHASSIS_STATUS_REQ)
    {
      if (fill_cmd_get_chassis_status (ip->obj_get_chassis_status_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_cmd_get_chassis_status: %s", strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_get_chassis_status_req;
    }
  else if (pkt == CHASSIS_CONTROL_REQ)
    {
      uint8_t command = 0;

      assert (ip->cmd == POWER_CMD_POWER_OFF
              || ip->cmd == POWER_CMD_POWER_ON
              || ip->cmd == POWER_CMD_POWER_CYCLE
              || ip->cmd == POWER_CMD_POWER_RESET
              || ip->cmd == POWER_CMD_PULSE_DIAG_INTR
              || ip->cmd == POWER_CMD_SOFT_SHUTDOWN_OS);

      if (ip->cmd == POWER_CMD_POWER_OFF)
        command = IPMI_CHASSIS_CONTROL_POWER_DOWN;
      else if (ip->cmd == POWER_CMD_POWER_ON)
        command = IPMI_CHASSIS_CONTROL_POWER_UP;
      else if (ip->cmd == POWER_CMD_POWER_CYCLE)
        command = IPMI_CHASSIS_CONTROL_POWER_CYCLE;
      else if (ip->cmd == POWER_CMD_POWER_RESET)
        command = IPMI_CHASSIS_CONTROL_HARD_RESET;
      else if (ip->cmd == POWER_CMD_PULSE_DIAG_INTR)
        command = IPMI_CHASSIS_CONTROL_PULSE_DIAGNOSTIC_INTERRUPT;
      else if (ip->cmd == POWER_CMD_SOFT_SHUTDOWN_OS)
        command = IPMI_CHASSIS_CONTROL_INITIATE_SOFT_SHUTDOWN;

      if (fill_cmd_chassis_control (command, ip->obj_chassis_control_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_cmd_chassis_control: %s", strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_chassis_control_req;
    }
  else if (pkt == CHASSIS_IDENTIFY_REQ)
    {
      uint8_t identify_interval;
      uint8_t force_identify;
      uint8_t *identify_interval_ptr = NULL;
      uint8_t *force_identify_ptr = NULL;

      assert (ip->cmd == POWER_CMD_IDENTIFY_ON
              || ip->cmd == POWER_CMD_IDENTIFY_OFF);

      if (ip->cmd == POWER_CMD_IDENTIFY_ON)
        {
          /* must pass interval for force to be taken */
          identify_interval = 0xFF;
          identify_interval_ptr = &identify_interval;

          force_identify = IPMI_CHASSIS_FORCE_IDENTIFY_ON;
          force_identify_ptr = &force_identify;
        }
      else
        {
          identify_interval = 0;
          identify_interval_ptr = &identify_interval;
        }

      if (fill_cmd_chassis_identify (identify_interval_ptr,
                                     force_identify_ptr,
                                     ip->obj_chassis_identify_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_cmd_chassis_identify: %s", strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_chassis_identify_req;
    }
  else if (pkt == CLOSE_SESSION_REQ)
    {
      if (fill_cmd_close_session (session_id,
                                  NULL,
                                  ip->obj_close_session_req) < 0)
        {
          IPMIPOWER_ERROR (("fill_cmd_close_session: %s", strerror (errno)));
          exit (1);
        }
      obj_cmd_req = ip->obj_close_session_req;
    }

  /* Construct packets */
  if (pkt == AUTHENTICATION_CAPABILITIES_REQ
      || pkt == GET_SESSION_CHALLENGE_REQ
      || pkt == ACTIVATE_SESSION_REQ
      || (cmd_args.common.driver_type == IPMI_DEVICE_LAN
          && (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ
              || pkt == GET_CHASSIS_STATUS_REQ
              || pkt == CHASSIS_CONTROL_REQ
              || pkt == CHASSIS_IDENTIFY_REQ
              || pkt == CLOSE_SESSION_REQ)))
    rv = _ipmi_1_5_packet_create (ip,
                                  pkt,
                                  authentication_type,
                                  sequence_number,
                                  session_id,
                                  password,
                                  (password) ? strlen (password) : 0,
                                  net_fn,
                                  obj_cmd_req,
                                  buf,
                                  buflen);
  else if (pkt == OPEN_SESSION_REQ
           || pkt == RAKP_MESSAGE_1_REQ
           || pkt == RAKP_MESSAGE_3_REQ
           || (cmd_args.common.driver_type == IPMI_DEVICE_LAN_2_0
               && (pkt == SET_SESSION_PRIVILEGE_LEVEL_REQ
                   || pkt == GET_CHASSIS_STATUS_REQ
                   || pkt == CHASSIS_CONTROL_REQ
                   || pkt == CHASSIS_IDENTIFY_REQ
                   || pkt == CLOSE_SESSION_REQ)))
    rv = _ipmi_2_0_packet_create (ip,
                                  pkt,
                                  payload_type,
                                  payload_authenticated,
                                  payload_encrypted,
                                  session_id,
                                  sequence_number,
                                  password,
                                  (password) ? strlen (password) : 0,
                                  net_fn,
                                  authentication_algorithm,
                                  integrity_algorithm,
                                  confidentiality_algorithm,
                                  integrity_key,
                                  integrity_key_len,
                                  confidentiality_key,
                                  confidentiality_key_len,
                                  obj_cmd_req,
                                  buf,
                                  buflen);
  else
    {
      IPMIPOWER_ERROR (("host = %s; p = %d; invalid logic",
                        ip->ic->hostname, ip->protocol_state));
      exit (1);
    }

  return (rv);
}

msg_type_t
ipmipower_packet_errmsg (ipmipower_powercmd_t ip, packet_type_t pkt)
{
  fiid_obj_t obj_cmd;

  assert (ip);
  assert (PACKET_TYPE_VALID_RES (pkt));

  obj_cmd = ipmipower_packet_cmd_obj (ip, pkt);

  if (pkt == OPEN_SESSION_RES
      || pkt == RAKP_MESSAGE_2_RES
      || pkt == RAKP_MESSAGE_4_RES)
    {
      uint8_t rmcpplus_status_code;
      uint64_t val;

      if (FIID_OBJ_GET (obj_cmd,
                        "rmcpplus_status_code",
                        &val) < 0)
        {
          IPMIPOWER_ERROR (("FIID_OBJ_GET: 'rmcpplus_status_code': %s",
                            fiid_obj_errormsg (obj_cmd)));
          exit (1);
        }
      rmcpplus_status_code = val;

      /* achu:

      At this point in time, my belief is that the following RMCPPLUS
      Status Codes:

      RMCPPLUS_STATUS_INVALID_AUTHENTICATION_ALGORITHM
      RMCPPLUS_STATUS_INVALID_INTEGRITY_ALGORITHM
      RMCPPLUS_STATUS_INVALID_CONFIDENTIALITY_ALGORITHM
      RMCPPLUS_STATUS_INVALID_ROLE
      RMCPPLUS_STATUS_NO_MATCHING_AUTHENTICATION_PAYLOAD
      RMCPPLUS_STATUS_NO_MATCHING_INTEGRITY_PAYLOAD

      Imply that an incorrect algorithm/role/payload value was sent.
      *NOT* an unsupported algorithm/role/payload.  I assume unsupported algorithm/role/payloads
      will get different error codes.

      If my assumption is later proven incorrect, then I need to redo some of this.

      */

      if (rmcpplus_status_code == RMCPPLUS_STATUS_NO_ERRORS)
        {
          IPMIPOWER_ERROR (("host = %s; p = %d; pkt = %d; called with good rmcpplus_status_code",
                            ip->ic->hostname, ip->protocol_state, pkt));
          exit (1);
        }
      else if (rmcpplus_status_code == RMCPPLUS_STATUS_INSUFFICIENT_RESOURCES_TO_CREATE_A_SESSION
               || rmcpplus_status_code == RMCPPLUS_STATUS_INSUFFICIENT_RESOURCES_TO_CREATE_A_SESSION_AT_THE_REQUESTED_TIME)
        return (MSG_TYPE_BMC_BUSY);
      else if (rmcpplus_status_code == RMCPPLUS_STATUS_UNAUTHORIZED_ROLE_OR_PRIVILEGE_LEVEL_REQUESTED
	       || rmcpplus_status_code == RMCPPLUS_STATUS_INVALID_ROLE)
        return (MSG_TYPE_PRIVILEGE_LEVEL_CANNOT_BE_OBTAINED);
      else if (rmcpplus_status_code == RMCPPLUS_STATUS_UNAUTHORIZED_NAME)
        return (MSG_TYPE_USERNAME_INVALID);
      else if (rmcpplus_status_code == RMCPPLUS_STATUS_NO_CIPHER_SUITE_MATCH_WITH_PROPOSED_SECURITY_ALGORITHMS)
        return (MSG_TYPE_CIPHER_SUITE_ID_UNAVAILABLE);
    }
  else
    {
      uint8_t comp_code;
      uint64_t val;

      if (FIID_OBJ_GET (obj_cmd,
                        "comp_code",
                        &val) < 0)
        {
          IPMIPOWER_ERROR (("FIID_OBJ_GET: 'comp_code': %s",
                            fiid_obj_errormsg (obj_cmd)));
          exit (1);
        }
      comp_code = val;

      if (comp_code == IPMI_COMP_CODE_COMMAND_SUCCESS)
        {
          IPMIPOWER_ERROR (("host = %s; p = %d; pkt = %d; called with good comp_code",
                            ip->ic->hostname, ip->protocol_state, pkt));
          exit (1);
        }
      else if (pkt == AUTHENTICATION_CAPABILITIES_RES
               && cmd_args.common.driver_type == IPMI_DEVICE_LAN_2_0
               && comp_code == IPMI_COMP_CODE_INVALID_DATA_FIELD_IN_REQUEST)
        return (MSG_TYPE_IPMI_2_0_UNAVAILABLE);
      else if (pkt == GET_SESSION_CHALLENGE_RES
               && (comp_code == IPMI_COMP_CODE_GET_SESSION_CHALLENGE_INVALID_USERNAME
                   || comp_code == IPMI_COMP_CODE_GET_SESSION_CHALLENGE_NULL_USERNAME_NOT_ENABLED))
        return (MSG_TYPE_USERNAME_INVALID);
      else if (pkt == ACTIVATE_SESSION_RES
               && comp_code == IPMI_COMP_CODE_ACTIVATE_SESSION_EXCEEDS_PRIVILEGE_LEVEL)
        return (MSG_TYPE_PRIVILEGE_LEVEL_CANNOT_BE_OBTAINED);
      else if (pkt == ACTIVATE_SESSION_RES
               && (comp_code == IPMI_COMP_CODE_ACTIVATE_SESSION_NO_SESSION_SLOT_AVAILABLE
                   || comp_code == IPMI_COMP_CODE_ACTIVATE_SESSION_NO_SLOT_AVAILABLE_FOR_GIVEN_USER
                   || comp_code == IPMI_COMP_CODE_ACTIVATE_SESSION_NO_SLOT_AVAILABLE_TO_SUPPORT_USER))
        return (MSG_TYPE_BMC_BUSY);
      else if (pkt == SET_SESSION_PRIVILEGE_LEVEL_RES
               && (comp_code == IPMI_COMP_CODE_SET_SESSION_PRIVILEGE_LEVEL_REQUESTED_LEVEL_NOT_AVAILABLE_FOR_USER
                   || comp_code == IPMI_COMP_CODE_SET_SESSION_PRIVILEGE_LEVEL_REQUESTED_LEVEL_EXCEEDS_USER_PRIVILEGE_LIMIT
                   || comp_code == IPMI_COMP_CODE_SET_SESSION_PRIVILEGE_LEVEL_CANNOT_DISABLE_USER_LEVEL_AUTHENTICATION))
        return (MSG_TYPE_PRIVILEGE_LEVEL_CANNOT_BE_OBTAINED);
#if 0
      /* Should not reach this point, should be handled by other code */
      else if (pkt == CHASSIS_CONTROL_RES
               && comp_code == IPMI_COMP_CODE_INSUFFICIENT_PRIVILEGE_LEVEL)
        return (MSG_TYPE_PRIVILEGE_LEVEL_INSUFFICIENT);
#endif
      else if (pkt == CHASSIS_CONTROL_RES
               && comp_code == IPMI_COMP_CODE_REQUEST_PARAMETER_NOT_SUPPORTED)
        return (MSG_TYPE_OPERATION_INVALID);
    }

  return (MSG_TYPE_BMC_ERROR);
}